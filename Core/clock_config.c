/*
 * Copyright 2019-2020, 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Adapted from the Variscite freertos-variscite SDK fork
 * (boards/som_mx8mp/driver_examples/flexcan/interrupt_transfer/clock_config.c,
 * branch mcuxpresso_sdk_2.15.x-var02): dropped the audio/SAI PLL config and
 * the RPMsg-audio domain power-up sequence (SUPPORT_RPMSG_AUDIO branch in
 * the original), none of which are needed for CAN-only bring-up. The M7/AHB
 * root and RDC/domain-1 clock gating logic is unchanged.
 */

#include "fsl_common.h"
#include "clock_config.h"

void BOARD_BootClockRUN(void)
{
    /*
     * The following steps just show how to configure the PLL clock sources
     * using the clock driver on the M7 core side. The ROM has already
     * configured SYSTEM PLL1 to 800MHz at power-up, and the A53 core enables
     * SYSTEM PLL1/2/3 via U-Boot before the M7 is kicked via remoteproc.
     * Re-configuring the system PLLs here risks hanging the SoC -- don't.
     */

    /* Switch AHB NOC root to 24M first in order to configure SYSTEM PLL1. */
    CLOCK_SetRootMux(kCLOCK_RootAhb, kCLOCK_AhbRootmuxOsc24M);

    /* Switch AXI M7 root to 24M first in order to configure SYSTEM PLL2. */
    CLOCK_SetRootMux(kCLOCK_RootM7, kCLOCK_M7RootmuxOsc24M);

    CLOCK_SetRootDivider(kCLOCK_RootM7, 1U, 1U);              /* Set root clock to 800M */
    CLOCK_SetRootMux(kCLOCK_RootM7, kCLOCK_M7RootmuxSysPll1); /* Switch Cortex-M7 to SYSTEM PLL1 */

    CLOCK_SetRootDivider(kCLOCK_RootAhb, 1U, 1U);                   /* Set root clock freq to 133MHz / 1 = 133MHz */
    CLOCK_SetRootMux(kCLOCK_RootAhb, kCLOCK_AhbRootmuxSysPll1Div6); /* Switch AHB to SYSTEM PLL1 DIV6 */

    CLOCK_EnableClock(kCLOCK_Rdc);   /* Enable RDC clock */
    CLOCK_EnableClock(kCLOCK_Ocram); /* Enable Ocram clock */

    /* Keep these enabled so the M7 core still works normally when the A53 core enters low power status. */
    CLOCK_EnableClock(kCLOCK_Sim_m);
    CLOCK_EnableClock(kCLOCK_Sim_main);
    CLOCK_EnableClock(kCLOCK_Sim_s);
    CLOCK_EnableClock(kCLOCK_Sim_wakeup);
    CLOCK_EnableClock(kCLOCK_Debug);
    CLOCK_EnableClock(kCLOCK_Dram);
    CLOCK_EnableClock(kCLOCK_Sec_Debug);

    /* Update core clock */
    SystemCoreClockUpdate();
}
