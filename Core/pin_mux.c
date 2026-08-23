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
}
