/*
 * Copyright 2020-2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Adapted from the Variscite freertos-variscite SDK fork
 * (boards/som_mx8mp/driver_examples/flexcan/interrupt_transfer/pin_mux.c,
 * branch mcuxpresso_sdk_2.15.x-var02): trimmed to the FLEXCAN2 pins only
 * (the reference example also configured UART4 for its debug console,
 * which this skeleton does not use).
 *
 * 2026-08-24: split into per-peripheral functions -- see pin_mux.h for why.
 * BOARD_InitPins() itself is now empty; callers mux each peripheral's pins
 * individually, only after that peripheral's RDC check has passed.
 */

#include "fsl_common.h"
#include "fsl_iomuxc.h"
#include "pin_mux.h"

void BOARD_InitBootPins(void)
{
    BOARD_InitPins();
}

void BOARD_InitPins(void)
{
}

/*
 * Pin assignment (from Variscite's som_mx8mp board file, coreID: cm7):
 *   AJ4 -> FLEXCAN2 can_rx, pin_signal UART3_TXD (pad shared with UART3_TXD on i.MX8M's IOMUX)
 *   AE6 -> FLEXCAN2 can_tx, pin_signal UART3_RXD (pad shared with UART3_RXD on i.MX8M's IOMUX)
 *
 * Independently re-confirmed 2026-08-24 against the live Symphony board's own schematic
 * (same bbox method used for I2C3/SPI/UART below) -- the carrier's actual CAN transceiver
 * traces to these exact balls, so this is the physically-correct peripheral. See main.c's
 * VCU_CanRdcGranted() for why this is gated: the live board's Linux device tree shows
 * FLEXCAN2 claimed and bound to a can0 interface.
 */
void BOARD_InitCanPins(void)
{
    IOMUXC_SetPinMux(IOMUXC_UART3_TXD_CAN2_RX, 0U);
    IOMUXC_SetPinConfig(IOMUXC_UART3_TXD_CAN2_RX,
                         IOMUXC_SW_PAD_CTL_PAD_DSE(1U) |
                         IOMUXC_SW_PAD_CTL_PAD_FSEL_MASK |
                         IOMUXC_SW_PAD_CTL_PAD_PUE_MASK |
                         IOMUXC_SW_PAD_CTL_PAD_PE_MASK);

    IOMUXC_SetPinMux(IOMUXC_UART3_RXD_CAN2_TX, 0U);
    IOMUXC_SetPinConfig(IOMUXC_UART3_RXD_CAN2_TX,
                         IOMUXC_SW_PAD_CTL_PAD_DSE(1U) |
                         IOMUXC_SW_PAD_CTL_PAD_FSEL_MASK |
                         IOMUXC_SW_PAD_CTL_PAD_PUE_MASK |
                         IOMUXC_SW_PAD_CTL_PAD_PE_MASK);
}

/*
 * I2C3 pins (alt0, native I2C3_SCL/I2C3_SDA pads, balls AJ7/AJ6) carry the
 * Symphony carrier's "I2C#A" bus. Derived by cross-referencing SOM edge
 * connector pins between two independent Variscite documents (see README.md):
 *   - Symphony-Board-Schematics.pdf, sheet "03. SOM": I2C#A_SCL/I2C#A_SDA
 *     land on SOM header pins 88/87.
 *   - VAR-SOM-MX8M-PLUS_Datasheet.pdf, Table 52 (I2C3 Signals): pin 88 =
 *     I2C3_SCL alt0 (ball AJ7), pin 87 = I2C3_SDA alt0 (ball AJ6).
 * I2C#A also carries the carrier's onboard PCA9534 GPIO expander (U5, addr
 * 0x20), which drives the D10 heartbeat LED but also SW1-4 and several
 * board-housekeeping signals (ENET1 reset, SOM voltage select) -- Linux/A53
 * very likely already owns this bus. See the RDC access check in main.c
 * before this pin mux is trusted to mean the M7 may safely drive it.
 * External 10K pull-ups already exist on the carrier (R19/R24-27), so only
 * open-drain mode is enabled here, not the pad's internal pull.
 */
void BOARD_InitI2c3Pins(void)
{
    IOMUXC_SetPinMux(IOMUXC_I2C3_SCL_I2C3_SCL, 0U);
    IOMUXC_SetPinConfig(IOMUXC_I2C3_SCL_I2C3_SCL,
                         IOMUXC_SW_PAD_CTL_PAD_DSE(1U) |
                         IOMUXC_SW_PAD_CTL_PAD_FSEL_MASK |
                         IOMUXC_SW_PAD_CTL_PAD_ODE_MASK);

    IOMUXC_SetPinMux(IOMUXC_I2C3_SDA_I2C3_SDA, 0U);
    IOMUXC_SetPinConfig(IOMUXC_I2C3_SDA_I2C3_SDA,
                         IOMUXC_SW_PAD_CTL_PAD_DSE(1U) |
                         IOMUXC_SW_PAD_CTL_PAD_FSEL_MASK |
                         IOMUXC_SW_PAD_CTL_PAD_ODE_MASK);
}

/*
 * Mikroe click expansion pins (see Core/expansion.c and the project's memory
 * file for the full derivation -- Symphony J16/J17/J30 cross-referenced
 * against the VAR-SOM-MX8M-PLUS datasheet's per-peripheral signal tables,
 * same method as I2C#A above).
 *
 * Wake input (OPTO 3 click IN1, via Symphony J17 pin 10 = SOM pin 79):
 * GPIO3_IO14, alt5 on the NAND_DQS pad. Active-low (both the OPTO click's
 * open-drain output and this pad idle high); external pull-up already
 * exists on the OPTO click, so no internal pull is enabled.
 */
void BOARD_InitWakeInputPin(void)
{
    IOMUXC_SetPinMux(IOMUXC_NAND_DQS_GPIO3_IO14, 0U);
    IOMUXC_SetPinConfig(IOMUXC_NAND_DQS_GPIO3_IO14, IOMUXC_SW_PAD_CTL_PAD_DSE(1U) | IOMUXC_SW_PAD_CTL_PAD_FSEL_MASK);
}

/*
 * ADC9 IRQ/MDAT input (MCP3564T-E/NC, via Symphony J17 pin 3 = SOM pin 84):
 * GPIO3_IO06, alt5 on the NAND_DATA00 pad.
 * ADC9 SPI (ECSPI2, via Symphony J16 pins 2/4/6/8 = SOM pins 43/39/41/45):
 * native/alt0 ECSPI2_SCLK/SS0/MISO/MOSI pads -- no on-SOM buffering quirk,
 * unlike I2C3's alt4 path.
 */
void BOARD_InitAdc9Pins(void)
{
    IOMUXC_SetPinMux(IOMUXC_NAND_DATA00_GPIO3_IO06, 0U);
    IOMUXC_SetPinConfig(IOMUXC_NAND_DATA00_GPIO3_IO06,
                         IOMUXC_SW_PAD_CTL_PAD_DSE(1U) | IOMUXC_SW_PAD_CTL_PAD_FSEL_MASK);

    IOMUXC_SetPinMux(IOMUXC_ECSPI2_SCLK_ECSPI2_SCLK, 0U);
    IOMUXC_SetPinConfig(IOMUXC_ECSPI2_SCLK_ECSPI2_SCLK,
                         IOMUXC_SW_PAD_CTL_PAD_DSE(1U) | IOMUXC_SW_PAD_CTL_PAD_FSEL_MASK |
                         IOMUXC_SW_PAD_CTL_PAD_PUE_MASK | IOMUXC_SW_PAD_CTL_PAD_PE_MASK);

    IOMUXC_SetPinMux(IOMUXC_ECSPI2_SS0_ECSPI2_SS0, 0U);
    IOMUXC_SetPinConfig(IOMUXC_ECSPI2_SS0_ECSPI2_SS0,
                         IOMUXC_SW_PAD_CTL_PAD_DSE(1U) | IOMUXC_SW_PAD_CTL_PAD_FSEL_MASK |
                         IOMUXC_SW_PAD_CTL_PAD_PUE_MASK | IOMUXC_SW_PAD_CTL_PAD_PE_MASK);

    IOMUXC_SetPinMux(IOMUXC_ECSPI2_MISO_ECSPI2_MISO, 0U);
    IOMUXC_SetPinConfig(IOMUXC_ECSPI2_MISO_ECSPI2_MISO,
                         IOMUXC_SW_PAD_CTL_PAD_DSE(1U) | IOMUXC_SW_PAD_CTL_PAD_FSEL_MASK |
                         IOMUXC_SW_PAD_CTL_PAD_PUE_MASK | IOMUXC_SW_PAD_CTL_PAD_PE_MASK);

    IOMUXC_SetPinMux(IOMUXC_ECSPI2_MOSI_ECSPI2_MOSI, 0U);
    IOMUXC_SetPinConfig(IOMUXC_ECSPI2_MOSI_ECSPI2_MOSI,
                         IOMUXC_SW_PAD_CTL_PAD_DSE(1U) | IOMUXC_SW_PAD_CTL_PAD_FSEL_MASK |
                         IOMUXC_SW_PAD_CTL_PAD_PUE_MASK | IOMUXC_SW_PAD_CTL_PAD_PE_MASK);
}

/* LIN1 UART (UART1, Symphony J18 pins 3/5 = SOM pins 124/175). */
void BOARD_InitLin1Pins(void)
{
    IOMUXC_SetPinMux(IOMUXC_UART1_TXD_UART1_TX, 0U);
    IOMUXC_SetPinConfig(IOMUXC_UART1_TXD_UART1_TX,
                         IOMUXC_SW_PAD_CTL_PAD_DSE(1U) | IOMUXC_SW_PAD_CTL_PAD_FSEL_MASK |
                         IOMUXC_SW_PAD_CTL_PAD_PUE_MASK | IOMUXC_SW_PAD_CTL_PAD_PE_MASK);

    IOMUXC_SetPinMux(IOMUXC_UART1_RXD_UART1_RX, 0U);
    IOMUXC_SetPinConfig(IOMUXC_UART1_RXD_UART1_RX,
                         IOMUXC_SW_PAD_CTL_PAD_DSE(1U) | IOMUXC_SW_PAD_CTL_PAD_FSEL_MASK |
                         IOMUXC_SW_PAD_CTL_PAD_PUE_MASK | IOMUXC_SW_PAD_CTL_PAD_PE_MASK);
}

/* LIN2 UART (UART4, Symphony J18 pins 7/9 = SOM pins 171/115). */
void BOARD_InitLin4Pins(void)
{
    IOMUXC_SetPinMux(IOMUXC_UART4_TXD_UART4_TX, 0U);
    IOMUXC_SetPinConfig(IOMUXC_UART4_TXD_UART4_TX,
                         IOMUXC_SW_PAD_CTL_PAD_DSE(1U) | IOMUXC_SW_PAD_CTL_PAD_FSEL_MASK |
                         IOMUXC_SW_PAD_CTL_PAD_PUE_MASK | IOMUXC_SW_PAD_CTL_PAD_PE_MASK);

    IOMUXC_SetPinMux(IOMUXC_UART4_RXD_UART4_RX, 0U);
    IOMUXC_SetPinConfig(IOMUXC_UART4_RXD_UART4_RX,
                         IOMUXC_SW_PAD_CTL_PAD_DSE(1U) | IOMUXC_SW_PAD_CTL_PAD_FSEL_MASK |
                         IOMUXC_SW_PAD_CTL_PAD_PUE_MASK | IOMUXC_SW_PAD_CTL_PAD_PE_MASK);
}

/*
 * Shared LIN CS/WAKE outputs (GPIO4_IO14/IO15) -- shared across both MCP2003B clicks
 * per the user's decision that both LIN buses always move in unison; see the
 * project's memory file for the tradeoff this accepts. Via Symphony J30 (Extension
 * Connector) pins 4/6 = SOM pins 56/55. J30 repurposes the second-Ethernet-PHY RGMII
 * bus via on-board isolation resistors -- NOT YET CONFIRMED those resistors are
 * actually populated on this board; see README.md.
 */
void BOARD_InitLinControlPins(void)
{
    IOMUXC_SetPinMux(IOMUXC_SAI1_TXD2_GPIO4_IO14, 0U);
    IOMUXC_SetPinConfig(IOMUXC_SAI1_TXD2_GPIO4_IO14,
                         IOMUXC_SW_PAD_CTL_PAD_DSE(1U) | IOMUXC_SW_PAD_CTL_PAD_FSEL_MASK);

    IOMUXC_SetPinMux(IOMUXC_SAI1_TXD3_GPIO4_IO15, 0U);
    IOMUXC_SetPinConfig(IOMUXC_SAI1_TXD3_GPIO4_IO15,
                         IOMUXC_SW_PAD_CTL_PAD_DSE(1U) | IOMUXC_SW_PAD_CTL_PAD_FSEL_MASK);
}
