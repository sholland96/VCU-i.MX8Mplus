/*
 * Copyright 2019-2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Adapted from the Variscite freertos-variscite SDK fork
 * (boards/som_mx8mp/driver_examples/flexcan/interrupt_transfer/board.h,
 * branch mcuxpresso_sdk_2.15.x-var02): dropped the debug-UART, GPC/MU, and
 * codec-I2C definitions -- this skeleton doesn't use any of them yet.
 */

#ifndef _BOARD_H_
#define _BOARD_H_

#include "clock_config.h"
#include "fsl_clock.h"

/*! @brief The board name */
#define BOARD_NAME        "VAR-SOM-MX8M-PLUS"
#define MANUFACTURER_NAME "Variscite"
#define BOARD_DOMAIN_ID   (1U)

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

void BOARD_InitMemory(void);
void BOARD_RdcInit(void);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* _BOARD_H_ */
