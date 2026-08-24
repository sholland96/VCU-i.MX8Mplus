/*
 * VCU-i.MX8Mplus -- M7-side building blocks for NXP's "M core running in
 * system suspend" architecture (AN13400). Original file for this project,
 * not derived from a Variscite/NXP example line-for-line.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _LOWPOWER_H_
#define _LOWPOWER_H_

#include <stdbool.h>

#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * @brief Bring up the MU (Message Unit) peripheral the M7 uses to wake Linux.
 *
 * Gated on RDC: returns false (and does not touch the MU) if the M7's RDC
 * domain hasn't been granted read+write access to MU1_B, same caution as
 * VCU_LedRdcGranted() in main.c. Unlike I2C3, this side of the MU is meant
 * to be M7-exclusive by design (the A53 side is a separate RDC peripheral,
 * MU1_A) -- but that hasn't been confirmed against real hardware yet.
 */
bool VCU_LowPower_Init(void);

/*!
 * @brief Switch the M7's own core clock to the 24MHz OSC (AN13400 Method 1).
 *
 * Mirrors the exact CLOCK_SetRootMux() call clock_config.c's
 * BOARD_BootClockRUN() already uses momentarily at boot before switching to
 * SYSTEM PLL1 -- so this specific mux transition is already proven safe in
 * this project, just held here rather than passed through immediately.
 * Method 1 keeps the M7 *running* at the reduced clock (not halted) so it
 * can keep polling CAN/relays/etc, matching the always-on-real-time-
 * controller role -- it does not by itself put anything to sleep.
 */
void VCU_LowPower_EnterStandbyClock(void);

/*! @brief Restore the M7 core clock to SYSTEM PLL1 (the normal run state). */
void VCU_LowPower_ExitStandbyClock(void);

/*!
 * @brief Fire the MU GIR wake signal that brings a suspended A53/Linux back up.
 *
 * Wraps MU_TriggerInterrupts(MUB, kMU_GenInt0InterruptTrigger) -- the exact
 * call AN13400 section 5.2.3 documents for this purpose.
 *
 * NOT SUFFICIENT BY ITSELF: for the A53 to actually respond to this while in
 * DSM, the GPC interrupt mask for the MU1 wake source must already be
 * cleared on the relevant core's IMR. AN13400 says this is normally done by
 * ATF automatically when an LPA flag is active (not used here -- this is
 * Method 1), and otherwise must be "enabled in the GPC module in the M core
 * application code" -- deliberately not implemented here. The exact
 * register (which of GPC's per-core IMR blocks, i.e. AN13400's
 * gpc_imr_offset[last_core]) needs to be confirmed against the i.MX 8M Plus
 * Reference Manual's GPC chapter and real hardware before it's safe to
 * guess at, and touching GPC/ATF territory was explicitly deferred until
 * hardware arrives -- see the project's memory file. Calling this function
 * before that piece exists will not crash anything, it will just not
 * actually wake a suspended A53.
 */
void VCU_LowPower_WakeLinux(void);

#if defined(__cplusplus)
}
#endif

#endif /* _LOWPOWER_H_ */
