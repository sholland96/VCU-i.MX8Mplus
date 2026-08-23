/*
 * Copyright 2020-2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Adapted from the Variscite freertos-variscite SDK fork
 * (boards/som_mx8mp/driver_examples/flexcan/interrupt_transfer/pin_mux.h,
 * branch mcuxpresso_sdk_2.15.x-var02): trimmed to the FLEXCAN2 pins only.
 */

#ifndef _PIN_MUX_H_
#define _PIN_MUX_H_

#include "board.h"

#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * @brief Calls initialization functions.
 */
void BOARD_InitBootPins(void);

/*!
 * @brief Configures pin routing and optionally pin electrical features.
 */
void BOARD_InitPins(void); /*!< Function assigned for the core: Cortex-M7F[m7] */

#if defined(__cplusplus)
}
#endif

#endif /* _PIN_MUX_H_ */
