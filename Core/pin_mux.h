/*
 * Copyright 2020-2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Adapted from the Variscite freertos-variscite SDK fork
 * (boards/som_mx8mp/driver_examples/flexcan/interrupt_transfer/pin_mux.h,
 * branch mcuxpresso_sdk_2.15.x-var02): trimmed to the FLEXCAN2 pins only.
 *
 * 2026-08-24: split into per-peripheral pin-mux functions -- confirmed via the
 * i.MX 8M Plus Reference Manual (RDC_PDAPn reset value = all domains full
 * access) that IOMUXC pin muxing is NOT restricted by the RDC gates
 * protecting each peripheral's register block, so a single unconditional
 * BOARD_InitPins() muxing everything at boot could silently reconfigure
 * pins Linux's own pinctrl driver is actively using. Each function below is
 * now only called (from main.c/expansion.c) after that specific
 * peripheral's own RDC check has passed -- see the project's memory file.
 */

#ifndef _PIN_MUX_H_
#define _PIN_MUX_H_

#include "board.h"

#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * @brief Calls initialization functions. Does NOT mux any pins itself --
 * see BOARD_InitPins() below.
 */
void BOARD_InitBootPins(void);

/*!
 * @brief Kept for structural symmetry with Variscite's naming convention.
 * Intentionally empty: pin muxing is RDC-gated per peripheral below, not
 * done unconditionally at boot.
 */
void BOARD_InitPins(void); /*!< Function assigned for the core: Cortex-M7F[m7] */

/*! @brief FLEXCAN2 pins (AJ4/AE6). Call only after VCU_CanRdcGranted(). */
void BOARD_InitCanPins(void);

/*! @brief I2C3/D10 LED pins (AJ7/AJ6). Call only after VCU_LedRdcGranted(). */
void BOARD_InitI2c3Pins(void);

/*! @brief Wake input pin (GPIO3_IO14). Call only after GPIO3 RDC access is confirmed. */
void BOARD_InitWakeInputPin(void);

/*! @brief ADC9 SPI (ECSPI2) + IRQ pin (GPIO3_IO06). Call only after ECSPI2/GPIO3 RDC access is confirmed. */
void BOARD_InitAdc9Pins(void);

/*! @brief LIN1 UART (UART1) pins. Call only after UART1 RDC access is confirmed. */
void BOARD_InitLin1Pins(void);

/*! @brief LIN2 UART (UART4) pins. Call only after UART4 RDC access is confirmed. */
void BOARD_InitLin4Pins(void);

/*! @brief Shared LIN CS/WAKE pins (GPIO4_IO14/IO15). Call only after GPIO4 RDC access is confirmed. */
void BOARD_InitLinControlPins(void);

#if defined(__cplusplus)
}
#endif

#endif /* _PIN_MUX_H_ */
