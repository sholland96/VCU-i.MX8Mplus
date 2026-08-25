/*
 * VCU-i.MX8Mplus -- M7-side bring-up for the Mikroe click-board expansion
 * hardware. Original file for this project, not derived from a
 * Variscite/NXP example line-for-line. Scope, caveats, and sourcing: see
 * expansion.h and the project's memory file.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_ecspi.h"
#include "fsl_gpio.h"
#include "fsl_rdc.h"
#include "fsl_uart.h"
#include "board.h"
#include "clock_config.h"
#include "expansion.h"
#include "pin_mux.h"

#define VCU_WAKE_GPIO      GPIO3
#define VCU_WAKE_GPIO_PIN  14U /* GPIO3_IO14 */
#define VCU_ADC9_IRQ_GPIO  GPIO3
#define VCU_ADC9_IRQ_PIN   6U /* GPIO3_IO06 */
#define VCU_LIN_CS_GPIO    GPIO4
#define VCU_LIN_CS_PIN     14U /* GPIO4_IO14 */
#define VCU_LIN_WAKE_GPIO  GPIO4
#define VCU_LIN_WAKE_PIN   15U /* GPIO4_IO15 */

#define VCU_ADC9_SPI          ECSPI2
#define VCU_ADC9_SPI_BAUDRATE 1000000U /* conservative placeholder, MCP3564 supports much higher */

#define VCU_LIN1_UART         UART1
#define VCU_LIN2_UART         UART4
#define VCU_LIN_UART_BAUDRATE 19200U /* common LIN rate; TBD pending final network design */

static bool VCU_RdcGranted(rdc_periph_t periph)
{
    return RDC_GetPeriphAccessPolicy(RDC, periph, BOARD_DOMAIN_ID) == kRDC_ReadWrite;
}

bool VCU_Expansion_WakeInputInit(void)
{
    gpio_pin_config_t config = {kGPIO_DigitalInput, 0U};

    if (!VCU_RdcGranted(kRDC_Periph_GPIO3))
    {
        return false;
    }

    BOARD_InitWakeInputPin();
    GPIO_PinInit(VCU_WAKE_GPIO, VCU_WAKE_GPIO_PIN, &config);
    return true;
}

bool VCU_Expansion_WakeInputAsserted(void)
{
    return GPIO_PinRead(VCU_WAKE_GPIO, VCU_WAKE_GPIO_PIN) == 0U;
}

bool VCU_Expansion_Adc9Init(void)
{
    ecspi_master_config_t config;
    gpio_pin_config_t irqConfig = {kGPIO_DigitalInput, 0U};

    if (!VCU_RdcGranted(kRDC_Periph_ECSPI2) || !VCU_RdcGranted(kRDC_Periph_GPIO3))
    {
        return false;
    }

    BOARD_InitAdc9Pins();

    CLOCK_SetRootMux(kCLOCK_RootEcspi2, kCLOCK_EcspiRootmuxOsc24M);
    CLOCK_SetRootDivider(kCLOCK_RootEcspi2, 1U, 1U);

    ECSPI_MasterGetDefaultConfig(&config);
    config.channel      = kECSPI_Channel0;
    config.baudRate_Bps = VCU_ADC9_SPI_BAUDRATE;
    ECSPI_MasterInit(VCU_ADC9_SPI, &config, CLOCK_GetClockRootFreq(kCLOCK_Ecspi2ClkRoot));

    GPIO_PinInit(VCU_ADC9_IRQ_GPIO, VCU_ADC9_IRQ_PIN, &irqConfig);
    return true;
}

bool VCU_Expansion_Adc9IrqPinHigh(void)
{
    return GPIO_PinRead(VCU_ADC9_IRQ_GPIO, VCU_ADC9_IRQ_PIN) != 0U;
}

static bool VCU_Expansion_UartInit(UART_Type *base, clock_root_control_t clockRoot, clock_root_t clockRootName)
{
    uart_config_t config;

    UART_GetDefaultConfig(&config);
    config.baudRate_Bps = VCU_LIN_UART_BAUDRATE;

    CLOCK_SetRootMux(clockRoot, kCLOCK_UartRootmuxOsc24M);
    CLOCK_SetRootDivider(clockRoot, 1U, 1U);

    return UART_Init(base, &config, CLOCK_GetClockRootFreq(clockRootName)) == kStatus_Success;
}

bool VCU_Expansion_Lin1Init(void)
{
    if (!VCU_RdcGranted(kRDC_Periph_UART1))
    {
        return false;
    }

    BOARD_InitLin1Pins();
    return VCU_Expansion_UartInit(VCU_LIN1_UART, kCLOCK_RootUart1, kCLOCK_Uart1ClkRoot);
}

bool VCU_Expansion_Lin2Init(void)
{
    if (!VCU_RdcGranted(kRDC_Periph_UART4))
    {
        return false;
    }

    BOARD_InitLin4Pins();
    return VCU_Expansion_UartInit(VCU_LIN2_UART, kCLOCK_RootUart4, kCLOCK_Uart4ClkRoot);
}

bool VCU_Expansion_LinControlInit(void)
{
    gpio_pin_config_t config = {kGPIO_DigitalOutput, 0U};

    if (!VCU_RdcGranted(kRDC_Periph_GPIO4))
    {
        return false;
    }

    BOARD_InitLinControlPins();
    GPIO_PinInit(VCU_LIN_CS_GPIO, VCU_LIN_CS_PIN, &config);
    GPIO_PinInit(VCU_LIN_WAKE_GPIO, VCU_LIN_WAKE_PIN, &config);
    return true;
}

void VCU_Expansion_LinSetCs(bool high)
{
    GPIO_PinWrite(VCU_LIN_CS_GPIO, VCU_LIN_CS_PIN, high ? 1U : 0U);
}

void VCU_Expansion_LinSetWake(bool high)
{
    GPIO_PinWrite(VCU_LIN_WAKE_GPIO, VCU_LIN_WAKE_PIN, high ? 1U : 0U);
}
