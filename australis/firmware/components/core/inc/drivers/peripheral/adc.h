/**
 * @author Matt Ricci
 * @addtogroup API API Reference
 * @{
 * @addtogroup ADC
 * @brief Analog to Digital Converter (ADC) peripheral driver.
 * @}
 */

// ALLOW FORMATTING
#ifndef ADC_H
#define ADC_H

#include "stm32f439xx.h"
#include "stdint.h"
#include "stdbool.h"

// Macro definitions for pin config literals
//

/**
 * @brief Default ADC configuration initializer.
 * more functional default.
 */
#define ADC_CONFIG_DEFAULT            \
  (ADC_Config) {                      \
    .RES     = ADC_RES_12,            \
    .AWDEN   = false,                 \
    .JAWDEN  = false,                 \
    .EOCIE   = false,                 \
    .AWDIE   = false,                 \
    .JEOCIE  = false,                 \
    .SCAN    = false,                 \
    .ALIGN   = ADC_ALIGN_RIGHT,       \
    .EOCS    = ADC_EOCS_SINGLE,       \
    .DMA     = false,                 \
    .CONT    = false,                 \
    .HTR     = 0,                     \
    .LTR     = 0,                     \
    .L       = 0,                     \
    .JL      = 0,                     \
    .TSVREFE = false,                 \
    .VBATE   = true,                  \
    .ADCPRE  = ADC_ADCPRE_PCLK2_DIV2, \
  }
// clang-format off

/**
 * @brief Mask for configurable bits in ADC Control Register 1 (CR1) via ADC_Config.
 * @details Used to isolate configuration settings related to CR1 that are
 *          managed by the ADC_Config structure.
 */
#define ADC_CR1_CONFIG_MASK ( \
    ADC_CR1_RES               \
  | ADC_CR1_AWDEN             \
  | ADC_CR1_JAWDEN            \
  | ADC_CR1_AWDSGL            \
  | ADC_CR1_SCAN              \
  | ADC_CR1_JEOCIE            \
  | ADC_CR1_AWDIE             \
  | ADC_CR1_EOCIE             \
  | ADC_CR1_AWDCH             \
)

/**
 * @brief Mask for configurable bits in ADC Control Register 2 (CR2) via ADC_Config.
 * @details Used to isolate configuration settings related to CR2 that are
 *          managed by the ADC_Config structure. (Excludes ADON, SWSTART etc. which are handled separately).
 */
#define ADC_CR2_CONFIG_MASK ( \
    ADC_CR2_ALIGN             \
  | ADC_CR2_EOCS              \
  | ADC_CR2_DMA               \
  | ADC_CR2_CONT              \
)

/**
 * @brief Mask for configurable bits in ADC Sample Time Register 2 (SMPR2) via ADC_Config.
 * @details Used to isolate configuration settings for channels 0-9 in SMPR2
 *          that are managed by the ADC_Config structure.
 */
#define ADC_SMPR2_CONFIG_MASK ( \
    ADC_SMPR2_SMP0              \
  | ADC_SMPR2_SMP1              \
  | ADC_SMPR2_SMP2              \
  | ADC_SMPR2_SMP3              \
  | ADC_SMPR2_SMP4              \
  | ADC_SMPR2_SMP5              \
  | ADC_SMPR2_SMP6              \
  | ADC_SMPR2_SMP7              \
  | ADC_SMPR2_SMP8              \
  | ADC_SMPR2_SMP9              \
)
/**
 * @brief Mask for configurable bits in ADC Sample Time Register 1 (SMPR1) via ADC_Config.
 * @details Used to isolate configuration settings for channels 10-18 in SMPR1
 *          that are managed by the ADC_Config structure.
 */
#define ADC_SMPR1_CONFIG_MASK ( \
    ADC_SMPR1_SMP10             \
  | ADC_SMPR1_SMP11             \
  | ADC_SMPR1_SMP12             \
  | ADC_SMPR1_SMP13             \
  | ADC_SMPR1_SMP14             \
  | ADC_SMPR1_SMP15             \
  | ADC_SMPR1_SMP16             \
  | ADC_SMPR1_SMP17             \
  | ADC_SMPR1_SMP18             \
)

/**
 * @brief Mask for configurable bits in ADC Regular Sequence Register 3 (SQR3) via ADC_Config.
 * @details Used to isolate configuration settings for regular sequence positions SQ1-SQ6
 *          that are managed by the ADC_Config structure.
 */
#define ADC_SQR3_CONFIG_MASK ( \
    ADC_SQR3_SQ1               \
  | ADC_SQR3_SQ2               \
  | ADC_SQR3_SQ3               \
  | ADC_SQR3_SQ4               \
  | ADC_SQR3_SQ5               \
  | ADC_SQR3_SQ6               \
)

/**
 * @brief Mask for configurable bits in ADC Regular Sequence Register 2 (SQR2) via ADC_Config.
 * @details Used to isolate configuration settings for regular sequence positions SQ7-SQ12
 *          that are managed by the ADC_Config structure.
 */
#define ADC_SQR2_CONFIG_MASK ( \
    ADC_SQR2_SQ7               \
  | ADC_SQR2_SQ8               \
  | ADC_SQR2_SQ9               \
  | ADC_SQR2_SQ10              \
  | ADC_SQR2_SQ11              \
  | ADC_SQR2_SQ12              \
)

/**
 * @brief Mask for configurable bits in ADC Regular Sequence Register 1 (SQR1) via ADC_Config.
 * @details Used to isolate configuration settings for regular sequence positions SQ13-SQ16
 *          and the sequence length (L) that are managed by the ADC_Config structure.
 */
#define ADC_SQR1_CONFIG_MASK ( \
    ADC_SQR1_SQ13              \
  | ADC_SQR1_SQ14              \
  | ADC_SQR1_SQ15              \
  | ADC_SQR1_SQ16              \
  | ADC_SQR1_L                 \
)

/**
 * @brief Mask for configurable bits in ADC Injected Sequence Register (JSQR) via ADC_Config.
 * @details Used to isolate configuration settings for injected sequence positions JSQ1-JSQ4
 *          and the injected sequence length (JL) that are managed by the ADC_Config structure.
 */
#define ADC_JSQR_CONFIG_MASK ( \
    ADC_JSQR_JSQ1              \
  | ADC_JSQR_JSQ2              \
  | ADC_JSQR_JSQ3              \
  | ADC_JSQR_JSQ4              \
  | ADC_JSQR_JL                \
)
// clang-format on

/**
 * @ingroup ADC
 * @addtogroup ADC_Interface Interface
 * @brief ADC interface for measuring external analog signals.
 * @{
 */

/**
 * @brief ADC input channels enumeration.
 * @details Defines the available analog input channels for the ADC.
 */
typedef enum {
  ADC_IN0,  //!< Analog input channel 0
  ADC_IN1,  //!< Analog input channel 1
  ADC_IN2,  //!< Analog input channel 2
  ADC_IN3,  //!< Analog input channel 3
  ADC_IN4,  //!< Analog input channel 4
  ADC_IN5,  //!< Analog input channel 5
  ADC_IN6,  //!< Analog input channel 6
  ADC_IN7,  //!< Analog input channel 7
  ADC_IN8,  //!< Analog input channel 8
  ADC_IN9,  //!< Analog input channel 9
  ADC_IN10, //!< Analog input channel 10
  ADC_IN11, //!< Analog input channel 11
  ADC_IN12, //!< Analog input channel 12
  ADC_IN13, //!< Analog input channel 13
  ADC_IN14, //!< Analog input channel 14
  ADC_IN15, //!< Analog input channel 15
  ADC_IN16, //!< Analog input channel 16
  ADC_IN17, //!< Analog input channel 17
  ADC_IN18, //!< Analog input channel 18
} ADC_Channel;

/**
 * @brief ADC conversion resolution enumeration.
 * @details Defines the number of bits for the ADC conversion result.
 * Higher resolution provides more precise measurements but may take longer.
 */
typedef enum {
  ADC_RES_12, //!< 12-bit resolution
  ADC_RES_10, //!< 10-bit resolution
  ADC_RES_8,  //!< 8-bit resolution
  ADC_RES_6,  //!< 6-bit resolution
} ADC_Resolution;

/**
 * @brief ADC data alignment enumeration.
 * @details Defines how the ADC conversion data is aligned within the 16-bit data register.
 */
typedef enum {
  ADC_ALIGN_RIGHT, //!< Data is right-aligned in the ADC_DR register
  ADC_ALIGN_LEFT,  //!< Data is left-aligned in the ADC_DR register
} ADC_Align;

/**
 * @brief ADC End of Conversion (EOC) selection enumeration.
 * @details Defines when the EOC (End Of Conversion) status flag is set.
 */
typedef enum {
  ADC_EOCS_SEQUENCE, //!< EOC flag is set at the end of a sequence of regular conversions
  ADC_EOCS_SINGLE,   //!< EOC flag is set at the end of each single regular conversion
} ADC_EocSelect;

/**
 * @brief ADC channel sample time enumeration.
 * @details Defines the duration for which the input voltage is sampled by the ADC.
 * Expressed in number of ADCCLK cycles. Longer sample times can improve accuracy
 * for high impedance sources.
 */
typedef enum {
  ADC_SMP_3,   //!< 3 ADC clock cycles
  ADC_SMP_15,  //!< 15 ADC clock cycles
  ADC_SMP_28,  //!< 28 ADC clock cycles
  ADC_SMP_56,  //!< 56 ADC clock cycles
  ADC_SMP_84,  //!< 84 ADC clock cycles
  ADC_SMP_112, //!< 112 ADC clock cycles
  ADC_SMP_144, //!< 144 ADC clock cycles
  ADC_SMP_480, //!< 480 ADC clock cycles
} ADC_SampleTime;

/**
 * @brief ADC regular channel sequence length enumeration.
 * @details Defines the number of conversions in the regular channel group sequence.
 * The value (e.g., ADC_L1 for 1 conversion) directly corresponds to the SQR1.L field value.
 * `ADC_L1` means 1 conversion, `ADC_L16` means 16 conversions.
 */
typedef enum {
  ADC_L1,  //!< 1 conversion in the regular sequence
  ADC_L2,  //!< 2 conversions in the regular sequence
  ADC_L3,  //!< 3 conversions in the regular sequence
  ADC_L4,  //!< 4 conversions in the regular sequence
  ADC_L5,  //!< 5 conversions in the regular sequence
  ADC_L6,  //!< 6 conversions in the regular sequence
  ADC_L7,  //!< 7 conversions in the regular sequence
  ADC_L8,  //!< 8 conversions in the regular sequence
  ADC_L9,  //!< 9 conversions in the regular sequence
  ADC_L10, //!< 10 conversions in the regular sequence
  ADC_L11, //!< 11 conversions in the regular sequence
  ADC_L12, //!< 12 conversions in the regular sequence
  ADC_L13, //!< 13 conversions in the regular sequence
  ADC_L14, //!< 14 conversions in the regular sequence
  ADC_L15, //!< 15 conversions in the regular sequence
  ADC_L16, //!< 16 conversions in the regular sequence
} ADC_SequenceLength;

/**
 * @brief ADC prescaler selection enumeration.
 * @details Defines the division factor for the ADC clock (ADCCLK) derived from PCLK2.
 */
typedef enum {
  ADC_ADCPRE_PCLK2_DIV2, //!< ADCCLK = PCLK2 divided by 2
  ADC_ADCPRE_PCLK2_DIV4, //!< ADCCLK = PCLK2 divided by 4
  ADC_ADCPRE_PCLK2_DIV6, //!< ADCCLK = PCLK2 divided by 6
  ADC_ADCPRE_PCLK2_DIV8, //!< ADCCLK = PCLK2 divided by 8
} ADC_Prescale;

/**
 * @brief ADC conversion type enumeration.
 * @details Specifies whether to start a regular or an injected conversion.
 */
typedef enum {
  ADC_CONVERSION_REGULAR, //!< Start a regular group conversion
  ADC_CONVERSION_INJECTED //!< Start an injected group conversion
} ADC_ConversionType;

/**
 * @brief ADC configuration structure.
 * @details Holds all configurable parameters for the ADC peripheral.
 *          This structure is used to initialize or update the ADC settings.
 */
typedef struct {
  ADC_Resolution RES;     //!< Resolution of the ADC (CR1.RES).
  bool AWDEN;             //!< Analog Watchdog Enable on regular channels (CR1.AWDEN).
  bool JAWDEN;            //!< Analog Watchdog Enable on injected channels (CR1.JAWDEN).
  bool EOCIE;             //!< Interrupt Enable for End Of regular Conversion (CR1.EOCIE).
  bool AWDIE;             //!< Interrupt Enable for Analog Watchdog (CR1.AWDIE).
  bool JEOCIE;            //!< Interrupt Enable for End Of Injected Conversion (CR1.JEOCIE).
  bool AWDSGL;            //!< Analog Watchdog on single channel (true) or all (false) (CR1.AWDSGL).
  bool SCAN;              //!< Scan mode enable for regular channels (CR1.SCAN).
  ADC_Channel AWDCH;      //!< Analog Watchdog Channel Select (if AWDSGL is true) (CR1.AWDCH).
  ADC_Align ALIGN;        //!< Data alignment (CR2.ALIGN).
  ADC_EocSelect EOCS;     //!< End Of Conversion Selection (CR2.EOCS).
  bool DMA;               //!< Direct Memory Access mode enable for regular channels (CR2.DMA).
  bool CONT;              //!< Continuous conversion mode for regular channels (CR2.CONT).
  ADC_SampleTime SMP[19]; //!< Sampling time for each channel (0-18) (SMPR1, SMPR2).
  uint16_t HTR;           //!< Analog watchdog higher threshold (HTR). Value should be 12-bit.
  uint16_t LTR;           //!< Analog watchdog lower threshold (LTR). Value should be 12-bit.
  ADC_SequenceLength L;   //!< Regular channel sequence length (1 to 16 conversions) (SQR1.L).
  ADC_Channel SQ[16];     //!< Regular channel sequence definition (SQ1 to SQ16) (SQR1, SQR2, SQR3).
  ADC_SequenceLength JL;  //!< Injected channel sequence length (1 to 4 conversions) (JSQR.JL).
  ADC_Channel JSQ[4];     //!< Injected channel sequence definition (JSQ1 to JSQ4) (JSQR).
  bool TSVREFE;           //!< Temperature sensor and VREFINT enable (ADC_CCR.TSVREFE).
  bool VBATE;             //!< VBAT channel enable (ADC_CCR.VBATE).
  ADC_Prescale ADCPRE;    //!< ADC prescaler for ADCCLK (ADC_CCR.ADCPRE).
} ADC_Config;

/**
 * @brief Struct definition for \ref ADC "ADC interface".
 * Provides the API handle for consumers to interact with an ADC peripheral instance.
 */
typedef struct _ADC {
  ADC_TypeDef *interface;                                             //!< Pointer to the STM32 ADC peripheral register map (e.g., ADC1).
  ADC_Config config;                                                  //!< Current configuration of the ADC peripheral.
  bool (*updateConfig)(struct _ADC *adc, ADC_Config *config);         //!< Function pointer to update the ADC configuration. @see ADC_updateConfig
  bool (*startConversion)(struct _ADC *adc, ADC_ConversionType type); //!< Function pointer to start an ADC conversion (regular or injected). @see ADC_startConversion
  uint16_t (*readData)(struct _ADC *adc);                             //!< Function pointer to read the ADC regular conversion result. @see ADC_readData
} ADC_t;

ADC_t ADC_init(ADC_TypeDef *adc_regs, ADC_Config *config);
bool ADC_startConversion(ADC_t *adc, ADC_ConversionType type);
uint16_t ADC_readData(ADC_t *adc);
bool ADC_updateConfig(ADC_t *adc, ADC_Config *config);

/** @} */
#endif
