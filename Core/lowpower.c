/*
 * VCU-i.MX8Mplus -- M7-side building blocks for NXP's "M core running in
 * system suspend" architecture (AN13400). Original file for this project,
 * not derived from a Variscite/NXP example line-for-line.
 *
 * Scope: AN13400 Method 1 only (24MHz-OSC clock source, no LPA flags/ATF
 * changes). None of this is called from main.c yet -- it's compiled and
 * verified but otherwise inert, matching the "write it, build-verify it,
 * flag as never-flashed" pattern already used for the CAN/I2C skeletons.
 * The starter kit hasn't arrived yet, and touching U-Boot/ATF/kernel
 * sources was explicitly deferred until it does -- see the project's
 * memory file for the full AN13400 findings and current status.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_mu.h"
#include "fsl_rdc.h"
#include "board.h"
#include "clock_config.h"
#include "lowpower.h"

bool VCU_LowPower_Init(void)
{
    if (RDC_GetPeriphAccessPolicy(RDC, kRDC_Periph_MU1_B, BOARD_DOMAIN_ID) != kRDC_ReadWrite)
    {
        return false;
    }

    MU_Init(MUB);
    return true;
}

void VCU_LowPower_EnterStandbyClock(void)
{
    /* Same mux transition BOARD_BootClockRUN() already performs momentarily at boot. */
    CLOCK_SetRootMux(kCLOCK_RootM7, kCLOCK_M7RootmuxOsc24M);
}

void VCU_LowPower_ExitStandbyClock(void)
{
    CLOCK_SetRootDivider(kCLOCK_RootM7, 1U, 1U);
    CLOCK_SetRootMux(kCLOCK_RootM7, kCLOCK_M7RootmuxSysPll1);
    SystemCoreClockUpdate();
}

void VCU_LowPower_WakeLinux(void)
{
    (void)MU_TriggerInterrupts(MUB, kMU_GenInt0InterruptTrigger);
}
