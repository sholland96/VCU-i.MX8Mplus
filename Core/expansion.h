/*
 * VCU-i.MX8Mplus -- M7-side bring-up for the Mikroe click-board expansion
 * hardware (OPTO 3 wake input, ADC 9 / MCP3564T-E-NC, 2x MCP2003B LIN).
 * Original file for this project, not derived from a Variscite/NXP example
 * line-for-line. None of the click boards are wired up yet -- see the
 * project's memory file for the full pin derivation. This is scaffolding
 * only: peripheral bring-up and raw pin-level access, no MCP3564 register
 * protocol and no LIN frame handling, matching Core/lowpower.c's inert
 * pattern (compiles and links, nothing calls it from main() yet).
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _EXPANSION_H_
#define _EXPANSION_H_

#include <stdbool.h>

#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * @brief Configure the wake input pin (GPIO3_IO14, Symphony J17 pin 10).
 * @return false if the M7's RDC domain hasn't been granted access to GPIO3.
 */
bool VCU_Expansion_WakeInputInit(void);

/*!
 * @brief Read the wake input. Active-low: returns true when the OPTO 3
 * click's IN1 is asserted (12V present pulls the opto-isolated, open-drain
 * signal low against its pull-up).
 */
bool VCU_Expansion_WakeInputAsserted(void);

/*!
 * @brief Bring up the ADC9 click's SPI bus (ECSPI2) and IRQ/MDAT input pin
 * (GPIO3_IO06, Symphony J17 pin 3).
 * @return false if the M7's RDC domain hasn't been granted access to
 * ECSPI2 or GPIO3. MCP3564 runs on its internal RC oscillator (no MCLK
 * wired) -- selecting that clock source is a CONFIG0 register write over
 * this SPI bus, not yet implemented here.
 */
bool VCU_Expansion_Adc9Init(void);

/*! @brief Raw IRQ/MDAT pin level. Polarity vs. the MCP3564's actual
 *  data-ready/interrupt behavior has not been confirmed against its
 *  datasheet (not yet in this project) -- do not assume active-low. */
bool VCU_Expansion_Adc9IrqPinHigh(void);

/*!
 * @brief Bring up LIN1 (UART1, Symphony J18 pins 3/5).
 * @return false if the M7's RDC domain hasn't been granted access to UART1.
 */
bool VCU_Expansion_Lin1Init(void);

/*!
 * @brief Bring up LIN2 (UART4, Symphony J18 pins 7/9).
 * @return false if the M7's RDC domain hasn't been granted access to UART4.
 */
bool VCU_Expansion_Lin2Init(void);

/*!
 * @brief Configure the CS and WAKE outputs shared by both MCP2003B LIN
 * clicks (GPIO4_IO14/IO15, Symphony J30 pins 4/6 -- NOT YET CONFIRMED the
 * carrier's isolation resistors for J30 are populated, see README.md).
 * @return false if the M7's RDC domain hasn't been granted access to GPIO4.
 */
bool VCU_Expansion_LinControlInit(void);

/*! @brief Drive the shared LIN CS line. Polarity vs. the MCP2003B's
 *  Normal/Standby mode select has not been confirmed against its
 *  datasheet (not yet in this project). */
void VCU_Expansion_LinSetCs(bool high);

/*! @brief Drive the shared LIN WAKE line. Same polarity caveat as CS. */
void VCU_Expansion_LinSetWake(bool high);

#if defined(__cplusplus)
}
#endif

#endif /* _EXPANSION_H_ */
