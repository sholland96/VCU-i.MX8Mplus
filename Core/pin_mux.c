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
 */

#include "fsl_common.h"
#include "fsl_iomuxc.h"
#include "pin_mux.h"

void BOARD_InitBootPins(void)
{
    BOARD_InitPins();
}

/*
 * Pin assignment (from Variscite's som_mx8mp board file, coreID: cm7):
 *   AJ4 -> FLEXCAN2 can_rx, pin_signal UART3_TXD (pad shared with UART3_TXD on i.MX8M's IOMUX)
 *   AE6 -> FLEXCAN2 can_tx, pin_signal UART3_RXD (pad shared with UART3_RXD on i.MX8M's IOMUX)
 *
 * NOT pin-traced with a meter against the Symphony v1.4a carrier's CAN
 * connector -- see README.md for the caveat.
 *
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
void BOARD_InitPins(void)
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
