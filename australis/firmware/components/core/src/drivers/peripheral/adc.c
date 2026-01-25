/***********************************************************************************
 * @file        adc.c                                                              *
 * @author      Matt Ricci                                                         *
 * @addtogroup  ADC                                                                *
 ***********************************************************************************/

#include "adc.h"

#include "stddef.h"

static void _ADC_init(ADC_TypeDef *, ADC_Config *);

/* =============================================================================== */
/**
 * @brief
 *
 * @param  interface
 * @param  config
 *
 * @return spi
 **
 * =============================================================================== */
ADC_t ADC_init(ADC_TypeDef *interface, ADC_Config *config) {
  // Early return error struct if peripheral is NULL
  if (interface == NULL) {
    return (ADC_t){.interface = NULL};
  }

  // Initialise ADC struct with interface
  ADC_t adc = {.interface = interface};

  // Update config and enable peripheral
  ADC_updateConfig(&adc, config);

  // Initialise methods
  adc.startConversion = ADC_startConversion;
  adc.readData        = ADC_readData;
  adc.updateConfig    = ADC_updateConfig;

  return adc;
}

// ALLOW FORMATTING
#ifndef __DOXYGEN__

/* =============================================================================== */
/**
 * @brief   Private initialiser for ADC registers.
 *
 * @param   interface Pointer to the ADC_TypeDef struct representing the ADC interface.
 * @param   config    Pointer to ADC_Config struct for initial configuration.
 *                    This may be passed as \c NULL to initialise a default
 *                    configuration. @see ADC_Config
 *
 * @return  @c NULL.
 **
 * =============================================================================== */
static void _ADC_init(ADC_TypeDef *interface, ADC_Config *config) {

  interface->CR2 &= ~ADC_CR2_ADON; // Power down ADC while configuring

  // --- Configure CR1 ---
  //
  // clang-format off
  interface->CR1 &= ~ADC_CR1_CONFIG_MASK;
  interface->CR1 |= (
      (config->RES    << ADC_CR1_RES_Pos)
    | (config->AWDEN  << ADC_CR1_AWDEN_Pos)
    | (config->JAWDEN << ADC_CR1_JAWDEN_Pos)
    | (config->AWDSGL << ADC_CR1_AWDSGL_Pos)
    | (config->SCAN   << ADC_CR1_SCAN_Pos)
    | (config->JEOCIE << ADC_CR1_JEOCIE_Pos)
    | (config->AWDIE  << ADC_CR1_AWDIE_Pos)
    | (config->EOCIE  << ADC_CR1_EOCIE_Pos)
    | (config->AWDCH  << ADC_CR1_AWDCH_Pos)
  );
  // clang-format on

  // --- Configure CR2 ---
  //
  // clang-format off
  interface->CR2 &= ~ADC_CR2_CONFIG_MASK;
  interface->CR2 |= (
      (config->ALIGN << ADC_CR2_ALIGN_Pos)
    | (config->EOCS  << ADC_CR2_EOCS_Pos)            
    | (config->DMA   << ADC_CR2_DMA_Pos)            
    | (config->CONT  << ADC_CR2_CONT_Pos)            
  );
  // clang-format on

  // --- Configure SMPR2 (Channels 10-18) and SMPR2 (Channels 0-9) ---
  // SMPR2: For channels 0 to 9
  interface->SMPR2 &= ~ADC_SMPR2_CONFIG_MASK;
  for (uint8_t i = 0; i < 10; i++) {
    interface->SMPR2 |= (config->SMP[i] << (i * 3));
  }

  // SMPR1: For channels 10 to 18
  interface->SMPR1 &= ~ADC_SMPR1_CONFIG_MASK;
  for (uint8_t i = 0; i < 9; i++) {
    interface->SMPR1 |= (config->SMP[10 + i] << (i * 3));
  }

  // --- Configure Watchdog Thresholds ---
  // HTR: Higher threshold. Only bits [11:0] are used.
  interface->HTR &= ~ADC_HTR_HT;
  interface->HTR |= config->HTR & ADC_HTR_HT_Msk;

  // LTR: Lower threshold. Only bits [11:0] are used.
  interface->LTR &= ~ADC_LTR_LT;
  interface->LTR |= config->LTR & ADC_LTR_LT_Msk;

  // --- Configure Regular Sequence Registers SQR1, SQR2, SQR3 ---
  // Number of conversions in regular sequence (L field in SQR1)
  // config->L is ADC_Lx which is 0 for 1 conversion, up to 15 for 16 conversions.
  // SQR1_L field is (number of conversions - 1).
  interface->SQR1 &= ~ADC_SQR1_CONFIG_MASK;
  interface->SQR1 |= (config->L << ADC_SQR1_L_Pos); // L is 0-indexed (0 means 1 conversion)

  // SQR3: SQ1-SQ6 (5 bits per SQx)
  interface->SQR3     &= ~ADC_SQR3_CONFIG_MASK;
  uint8_t conversions  = config->L + 1; // Actual number of conversions
  for (uint8_t i = 0; i < 6 && i < conversions; i++) {
    interface->SQR3 |= (config->SQ[i] << (i * 5));
  }

  // SQR2: SQ7-SQ12
  interface->SQR2 &= ~ADC_SQR2_CONFIG_MASK;
  for (uint8_t i = 0; i < 6 && (i + 6) < conversions; i++) {
    interface->SQR2 |= (config->SQ[i + 6] << (i * 5));
  }

  // SQR1: SQ13-SQ16
  interface->SQR1;
  for (uint8_t i = 0; i < 4 && (i + 12) < conversions; i++) {
    interface->SQR1 |= (config->SQ[i + 12] << (i * 5));
  }

  // --- Configure Injected Sequence (JSQR) ---
  interface->JSQR &= ~ADC_JSQR_CONFIG_MASK;
  interface->JSQR |= (config->JL << ADC_JSQR_JL_Pos); // JL is 0 for 1 conv, up to 3 for 4 convs.
  conversions      = config->JL + 1;
  for (uint8_t i = 0; i < 4 && i < conversions; i++) {
    interface->JSQR |= (config->JSQ[i] << (i * 4));
  }

  // clang-format off
  ADC123_COMMON->CCR &= ~(ADC_CCR_TSVREFE_Msk | ADC_CCR_VBATE_Msk | ADC_CCR_ADCPRE_Msk);
  ADC123_COMMON->CCR |= (
      ((config->TSVREFE & 1) << ADC_CCR_TSVREFE_Pos)
    | ((config->VBATE & 1)   << ADC_CCR_VBATE_Pos)
    | ((config->ADCPRE & 3)  << ADC_CCR_ADCPRE_Pos)
  );
  // clang-format on

  interface->CR2 |= ADC_CR2_ADON; // Finish configuring and power on
  for (volatile int i = 0; i < 500; ++i);
}

#endif

/* =============================================================================== */
/**
 * @brief   Starts a regular ADC conversion.
 * @details Sets the SWSTART (or JWSTART) bit in ADC_CR2 register to begin a conversion
 *          for the regular group. For continuous mode, this starts the sequence.
 *          For single mode, it starts one conversion.
 *
 * @param   adc Pointer to the ADC_t handle.
 * @return  void
 **
 * =============================================================================== */
bool ADC_startConversion(ADC_t *adc, ADC_ConversionType type) {
  // Eearly exit if struct is invalid
  if (adc == NULL || adc->interface == NULL) {
    return false;
  }

  // Ensure ADON is set
  if ((adc->interface->CR2 & ADC_CR2_ADON) == 0) {
    adc->interface->CR2 |= ADC_CR2_ADON;
  }

  // Set start bit and begin conversion
  if (type == ADC_CONVERSION_REGULAR) {
    adc->interface->CR2 |= ADC_CR2_SWSTART;
  } else if (type == ADC_CONVERSION_INJECTED) {
    adc->interface->CR2 |= ADC_CR2_JSWSTART;
  } else {
    return false;
  }

  return true;
}

/* =============================================================================== */
/**
 * @brief   Reads the data from the last ADC regular conversion.
 * @details Polls the EOC (End Of Conversion) flag and then returns the content
 *          of the ADC_DR register.
 *
 * @param   adc Pointer to the ADC_t handle.
 * @return  The ADC conversion data.
 **
 * =============================================================================== */
uint16_t ADC_readData(ADC_t *adc) {
  // Early exit with 0 if struct is invalid
  if (adc == NULL || adc->interface == NULL) {
    return 0;
  }

  // Wait for EOC (End Of Conversion) flag for regular channel
  // This is a blocking wait.
  while (!(adc->interface->SR & ADC_SR_EOC));

  // Reading DR clears the EOC flag (if EOCS=0 in CR2, or if it's the last of sequence)
  return (adc->interface->DR & 0xFFFF);
}

/* =============================================================================== */
/**
 * @brief   Update ADC peripheral configuration
 * @details Uses the provided configuration to update the ADC registers
 *          As with initialisation, passing \c NULL will set the default config.
 *
 * @param   adc Pointer to ADC_t struct.
 *
 * @return  @c NULL.
 **
 * =============================================================================== */
bool ADC_updateConfig(ADC_t *adc, ADC_Config *config) {
  // Initialise config with default values if passed NULL.
  if (config == NULL) {
    config = &ADC_CONFIG_DEFAULT;
  }

  // TODO: Validate config before setting, early return false on error

  // Update peripheral with new config
  adc->config = *config;

  // Initialise SPI registers and enable peripheral
  _ADC_init(adc->interface, config);

  return true;
}
