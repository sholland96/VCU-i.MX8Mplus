/*
 * VCU-i.MX8Mplus -- minimal bare-metal CAN bring-up skeleton for the
 * i.MX 8M Plus's Cortex-M7 core, on the Variscite Symphony v1.4a carrier +
 * VAR-SOM-MX8M-PLUS.
 *
 * Scope: CAN bring-up only. No RTOS, no Arduino/mbed ecosystem -- a plain
 * superloop, matching the architecture of the Teensy VCU project this is
 * eventually meant to host real-time control logic for. See ../README.md
 * for full sourcing/verification status before touching this file: this
 * skeleton compiles and links cleanly but has never been flashed to or run
 * on real hardware.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_common.h"
#include "fsl_flexcan.h"
#include "fsl_i2c.h"
#include "fsl_rdc.h"
#include "board.h"
#include "clock_config.h"
#include "pin_mux.h"

/* Variscite's som_mx8mp board file uses FLEXCAN2, not FLEXCAN1. */
#define VCU_CAN               FLEXCAN2
#define VCU_CAN_BITRATE       500000U /* classic CAN, matches the Teensy VCU's bus speed */
#define VCU_CAN_RX_MB_IDX     1U
#define VCU_CAN_TX_MB_IDX     0U

/*
 * D10 heartbeat LED, wired to P0 of U5 (PCA9534PWR I2C GPIO expander) on the
 * Symphony carrier's "I2C#A" bus -- see pin_mux.c for the pin derivation.
 * The same expander also drives SW1-4 and board-housekeeping signals Linux
 * almost certainly still owns, so this code checks RDC access to I2C3
 * before touching it at all rather than assuming the bus is free -- see
 * VCU_LedRdcGranted() below. Register-modifying writes only ever flip the
 * P0 bit via read-modify-write, never blast the whole register, so any
 * other pin state the expander is already holding (buttons, resets, etc.)
 * is left alone. NEVER FLASHED/VERIFIED -- see README.md.
 */
#define VCU_LED_I2C                 I2C3
#define VCU_LED_I2C_BAUDRATE        100000U /* PCA9534 supports up to 400kHz; staying conservative */
#define VCU_LED_I2C_ADDR            0x20U   /* PCA9534PWR, A0-A2 all grounded on this carrier */
#define VCU_LED_REG_OUTPUT          0x01U
#define VCU_LED_REG_CONFIG          0x03U
#define VCU_LED_P0_MASK             0x01U
#define VCU_LED_HEARTBEAT_PERIOD_MS 500U

static volatile uint32_t s_msTicks = 0U;

void SysTick_Handler(void)
{
    s_msTicks++;
}

static status_t VCU_Pca9534ReadReg(uint8_t reg, uint8_t *value)
{
    i2c_master_transfer_t xfer = {0};

    xfer.slaveAddress   = VCU_LED_I2C_ADDR;
    xfer.direction      = kI2C_Read;
    xfer.subaddress     = reg;
    xfer.subaddressSize = 1U;
    xfer.data           = value;
    xfer.dataSize       = 1U;

    return I2C_MasterTransferBlocking(VCU_LED_I2C, &xfer);
}

static status_t VCU_Pca9534WriteReg(uint8_t reg, uint8_t value)
{
    i2c_master_transfer_t xfer = {0};

    xfer.slaveAddress   = VCU_LED_I2C_ADDR;
    xfer.direction      = kI2C_Write;
    xfer.subaddress     = reg;
    xfer.subaddressSize = 1U;
    xfer.data           = &value;
    xfer.dataSize       = 1U;

    return I2C_MasterTransferBlocking(VCU_LED_I2C, &xfer);
}

/*
 * Only proceed with I2C3/D10 if the RDC has actually granted the M7's
 * domain read+write access to it. BOARD_RdcInit() only moves the M7 core
 * itself into domain 1 -- it does NOT reassign individual peripherals, so
 * whether I2C3 is actually available here depends entirely on whether
 * Linux/U-Boot's own device tree has released it. Treat "not granted" as
 * "Linux still owns this bus" and silently skip the LED rather than risk
 * bus contention with whatever is already using the GPIO expander.
 */
static bool VCU_LedRdcGranted(void)
{
    return RDC_GetPeriphAccessPolicy(RDC, kRDC_Periph_I2C3, BOARD_DOMAIN_ID) == kRDC_ReadWrite;
}

/*
 * FLEXCAN2 kernel clock: sourced from SYSTEM PLL1 (800MHz, already running --
 * see clock_config.c), divided by 2*5=10 down to 80MHz. Computed at runtime
 * rather than hardcoded, matching the pattern in Variscite's own reference
 * example (boards/som_mx8mp/driver_examples/flexcan/interrupt_transfer).
 */
#define VCU_CAN_CLK_FREQ                                                                \
    (CLOCK_GetPllFreq(kCLOCK_SystemPll1Ctrl) / CLOCK_GetRootPreDivider(kCLOCK_RootFlexCan2) / \
     CLOCK_GetRootPostDivider(kCLOCK_RootFlexCan2))

int main(void)
{
    flexcan_config_t flexcanConfig;
    flexcan_rx_mb_config_t rxMbConfig;
    flexcan_timing_config_t timingConfig;
    flexcan_frame_t rxFrame;
    i2c_master_config_t i2cConfig;
    bool ledAvailable   = false;
    uint32_t lastLedTick = 0U;

    /*
     * M7 has its own local cache; the smart-subsystem address range
     * (0x28000000-0x3FFFFFFF) must be marked non-cacheable before it's
     * accessed. Must run before any peripheral access.
     */
    BOARD_InitMemory();

    /* Put the M7 in its own RDC domain before touching shared peripherals. */
    BOARD_RdcInit();

    BOARD_InitBootPins();
    BOARD_BootClockRUN();

    /* Set FLEXCAN2 source to SYSTEM PLL1, divide 800MHz / (2*5) = 80MHz. */
    CLOCK_SetRootMux(kCLOCK_RootFlexCan2, kCLOCK_FlexCanRootmuxSysPll1);
    CLOCK_SetRootDivider(kCLOCK_RootFlexCan2, 2U, 5U);

    FLEXCAN_GetDefaultConfig(&flexcanConfig);
    flexcanConfig.bitRate = VCU_CAN_BITRATE;

    if (FLEXCAN_CalculateImprovedTimingValues(VCU_CAN, flexcanConfig.bitRate, VCU_CAN_CLK_FREQ, &timingConfig))
    {
        flexcanConfig.timingConfig = timingConfig;
    }

    FLEXCAN_Init(VCU_CAN, &flexcanConfig, VCU_CAN_CLK_FREQ);

    /* Accept-all mask on the Rx mailbox -- real ID filtering comes with the ported VCU logic. */
    FLEXCAN_SetRxMbGlobalMask(VCU_CAN, FLEXCAN_RX_MB_STD_MASK(0, 0, 0));

    rxMbConfig.format = kFLEXCAN_FrameFormatStandard;
    rxMbConfig.type   = kFLEXCAN_FrameTypeData;
    rxMbConfig.id     = FLEXCAN_ID_STD(0);
    FLEXCAN_SetRxMbConfig(VCU_CAN, VCU_CAN_RX_MB_IDX, &rxMbConfig, true);

    FLEXCAN_SetTxMbConfig(VCU_CAN, VCU_CAN_TX_MB_IDX, true);

    /* 1ms tick for the heartbeat LED's non-blocking timing -- see SysTick_Handler above. */
    SysTick_Config(CLOCK_GetClockRootFreq(kCLOCK_M7ClkRoot) / 1000U);

    ledAvailable = VCU_LedRdcGranted();
    if (ledAvailable)
    {
        uint8_t configReg;

        CLOCK_SetRootMux(kCLOCK_RootI2c3, kCLOCK_I2cRootmuxOsc24M);
        CLOCK_SetRootDivider(kCLOCK_RootI2c3, 1U, 1U);

        I2C_MasterGetDefaultConfig(&i2cConfig);
        i2cConfig.baudRate_Bps = VCU_LED_I2C_BAUDRATE;
        I2C_MasterInit(VCU_LED_I2C, &i2cConfig, CLOCK_GetClockRootFreq(kCLOCK_I2c3ClkRoot));

        /* Read-modify-write: only claim P0 (the LED) as an output, leave every other pin
         * (buttons, resets, SOM VSELECT, ...) exactly as Linux configured it. */
        if (VCU_Pca9534ReadReg(VCU_LED_REG_CONFIG, &configReg) == kStatus_Success)
        {
            configReg &= (uint8_t)~VCU_LED_P0_MASK;
            ledAvailable = (VCU_Pca9534WriteReg(VCU_LED_REG_CONFIG, configReg) == kStatus_Success);
        }
        else
        {
            ledAvailable = false;
        }
    }

    while (true)
    {
        /*
         * Poll the Rx mailbox, matching the Teensy VCU's no-RTOS superloop
         * architecture (no interrupt handles). FLEXCAN_ReadRxMb re-arms the
         * mailbox for the next frame on success.
         */
        if (FLEXCAN_ReadRxMb(VCU_CAN, VCU_CAN_RX_MB_IDX, &rxFrame) == kStatus_Success)
        {
            /* TODO: hand rxFrame off to the ported VCU CAN-message logic. */
            (void)rxFrame;
        }

        /* Non-blocking: only touch the LED once per heartbeat period, never stall CAN polling. */
        if (ledAvailable && (s_msTicks - lastLedTick) >= VCU_LED_HEARTBEAT_PERIOD_MS)
        {
            uint8_t outputReg;

            lastLedTick = s_msTicks;
            if (VCU_Pca9534ReadReg(VCU_LED_REG_OUTPUT, &outputReg) == kStatus_Success)
            {
                outputReg ^= VCU_LED_P0_MASK;
                (void)VCU_Pca9534WriteReg(VCU_LED_REG_OUTPUT, outputReg);
            }
        }
    }
}
