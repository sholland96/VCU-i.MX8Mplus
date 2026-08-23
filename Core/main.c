/*
 * VCU-i.MX8Mplus -- minimal bare-metal CAN bring-up skeleton for the
 * i.MX 8M Plus's Cortex-M7 core, on the Variscite Symphony v1.4a carrier +
 * VAR-SOM-MX8M-PLUS.
 *
 * Scope: CAN bring-up only. No RTOS, no Arduino/mbed ecosystem -- a plain
 * superloop, matching the architecture of the Teensy VCU project this is
 * eventually meant to host real-time control logic for. See ../README.md
 * for full sourcing/verification status before touching this file: this
 * skeleton has never been compiled or flashed to hardware.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_common.h"
#include "fsl_flexcan.h"
#include "board.h"
#include "clock_config.h"
#include "pin_mux.h"

/* Variscite's som_mx8mp board file uses FLEXCAN2, not FLEXCAN1. */
#define VCU_CAN               FLEXCAN2
#define VCU_CAN_BITRATE       500000U /* classic CAN, matches the Teensy VCU's bus speed */
#define VCU_CAN_RX_MB_IDX     1U
#define VCU_CAN_TX_MB_IDX     0U

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

    while (true)
    {
        /*
         * Poll the Rx mailbox, matching the polling-superloop style of the
         * Portenta X8 skeleton this project mirrors (no RTOS, no interrupt
         * handles). FLEXCAN_ReadRxMb re-arms the mailbox for the next frame
         * on success.
         */
        if (FLEXCAN_ReadRxMb(VCU_CAN, VCU_CAN_RX_MB_IDX, &rxFrame) == kStatus_Success)
        {
            /* TODO: hand rxFrame off to the ported VCU CAN-message logic. */
            (void)rxFrame;
        }
    }
}
