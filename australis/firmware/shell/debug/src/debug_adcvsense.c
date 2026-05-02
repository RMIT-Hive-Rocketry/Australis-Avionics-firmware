/***********************************************************************************
 * @file        debug_adcvsense.c                                                  *
 * @author      Matt Ricci                                                         *
 * @addtogroup  Shell                   																					 *
 *                                                                                 *
 * @{                                                                              *
 ***********************************************************************************/

#include "string.h"
#include "stdio.h"

#include "adc.h"
#include "gpiopin.h"
#include "rcc.h"
#include "shell.h"

static void ADCvsense_exec(UART_t *uart, char *);

DEFINE_PROGRAM_HANDLE("adc_vsense", ADCvsense_exec, NULL)

/* =============================================================================== */
/**
 * @brief
 *
 *
 **
 * =============================================================================== */
static void ADCvsense_exec(UART_t *uart, char *flags) {

  RCC_ENABLE_PERIPHERAL(APB2, ADC1);
  RCC_RESET_PERIPHERAL(APB2, ADC);

  GPIO_Config gpioConfig = GPIO_CONFIG_DEFAULT;
  gpioConfig.mode        = GPIO_MODE_ANALOG;

  GPIOpin_t pin          = GPIOpin_init(GPIOF, 3, &gpioConfig);
  uint16_t v             = 0;

  ADC_Config config      = ADC_CONFIG_DEFAULT;
  config.TSVREFE         = true;
  config.VBATE           = false;
  config.EOCS            = false;
  config.ADCPRE          = ADC_ADCPRE_PCLK2_DIV8;
  config.SQ[0]           = ADC_IN17;
  config.SMP[17]         = ADC_SMP_480;
  ADC_t vsense           = ADC_init(ADC1, &config);

  vsense.startConversion(&vsense, ADC_CONVERSION_REGULAR);
  v = vsense.readData(&vsense);
}

/** @} */
