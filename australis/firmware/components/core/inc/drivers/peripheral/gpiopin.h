/**
 * @author Matt Ricci
 * @addtogroup API API Reference
 * @{
 * @addtogroup GPIO
 * @}
 */

// ALLOW FORMATTING
#ifndef GPIOPIN_H
#define GPIOPIN_H

#include "stm32f439xx.h"
#include "stdbool.h"

#define GPIO_PERIPHERAL_SIZE 0x3FF // Size of each GPIO in memory

// Macro definitions for pin config literals
//
// clang-format off
#define GPIO_CONFIG_DEFAULT \
  (GPIO_Config) {           \
    GPIO_MODE_OUTPUT,       \
    GPIO_TYPE_PUSHPULL,     \
    GPIO_SPEED_HIGH,        \
    GPIO_PUPD_NONE,         \
    GPIO_AF0                \
  }
#define GPIO_CONFIG_INPUT   \
  (GPIO_Config) {           \
    GPIO_MODE_INPUT,        \
    GPIO_TYPE_PUSHPULL,     \
    GPIO_SPEED_HIGH,        \
    GPIO_PUPD_PULLUP,       \
    GPIO_AF0                \
  }
// clang-format on

/**
 * @ingroup GPIO
 * @addtogroup GPIO_Pin GPIO Pin
 * @brief Driver for initialising and interacting with individual GPIO pins.
 * @{
 */

/**
 * @brief   GPIO pin enum
 * @details Describes the position of the pin within its GPIO port.
 */
typedef enum {
  GPIO_PIN0,  //!< Pin 0
  GPIO_PIN1,  //!< Pin 1
  GPIO_PIN2,  //!< Pin 2
  GPIO_PIN3,  //!< Pin 3
  GPIO_PIN4,  //!< Pin 4
  GPIO_PIN5,  //!< Pin 5
  GPIO_PIN6,  //!< Pin 6
  GPIO_PIN7,  //!< Pin 7
  GPIO_PIN8,  //!< Pin 8
  GPIO_PIN9,  //!< Pin 9
  GPIO_PIN10, //!< Pin 10
  GPIO_PIN11, //!< Pin 11
  GPIO_PIN12, //!< Pin 12
  GPIO_PIN13, //!< Pin 13
  GPIO_PIN14, //!< Pin 14
  GPIO_PIN15  //!< Pin 15
} GPIO_Pin;

/**
 * @brief   GPIO alternate function enum
 * @details Describes the alternate function mapping of the pin.
 */
typedef enum {
  GPIO_AF0,  //!< System
  GPIO_AF1,  //!< TIM1/TIM2
  GPIO_AF2,  //!< TIM3..5
  GPIO_AF3,  //!< TIM8..11
  GPIO_AF4,  //!< I2C1..3
  GPIO_AF5,  //!< SPI1/2/3/4/5/6
  GPIO_AF6,  //!< SPI2/3/SAI1
  GPIO_AF7,  //!< USART1..3
  GPIO_AF8,  //!< USART4..8
  GPIO_AF9,  //!< CAN1/CAN2, LTDC, TIM12..14
  GPIO_AF10, //!< OTG_FS, OTG_HS
  GPIO_AF11, //!< ETH
  GPIO_AF12, //!< FMC, SDIO, OTG_HS
  GPIO_AF13, //!< DCMI
  GPIO_AF14, //!< LTDC
  GPIO_AF15  //!< EVENTOUT
} GPIO_AF;

/**
 * @brief   GPIO mode enum
 * @details Describes the I/O direction mode of the pin.
 */
typedef enum {
  GPIO_MODE_INPUT,  //!< Input mode (reset state)
  GPIO_MODE_OUTPUT, //!< General purpose output mode
  GPIO_MODE_AF,     //!< Alternate function mode
  GPIO_MODE_ANALOG  //!< Analog mode
} GPIO_Mode;

/**
 * @brief   GPIO type enum
 * @details Describes the output type of the pin.
 */
typedef enum {
  GPIO_TYPE_PUSHPULL,  //!< Push pull (reset state)
  GPIO_TYPE_OPENDRAIN, //!< Open drain
} GPIO_Type;

/**
 * @brief   GPIO speed enum
 * @details Describes the output speed of the pin.
 */
typedef enum {
  GPIO_SPEED_LOW,     //!< Low speed output       | (default for all not specified)
  GPIO_SPEED_MEDIUM,  //!< Medium speed output    |
  GPIO_SPEED_HIGH,    //!< High speed output      |
  GPIO_SPEED_VERYHIGH //!< Very high speed output | (default for PA13, PB3)
} GPIO_Speed;

/**
 * @brief   GPIO pull-up/pull-down enum
 * @details Describes if internal pull-up/pull-down is used by the pin.
 */
typedef enum {
  GPIO_PUPD_NONE,     //!< No pull-up, pull-down | (default for all not specified)
  GPIO_PUPD_PULLUP,   //!< Pull-up               | (default for PA15, PA13, PB4)
  GPIO_PUPD_PULLDOWN, //!< Pull-down             | (default for PB14)
} GPIO_PUPD;

/**
 * @brief   Struct definition for GPIO configuration.
 * @details Describes the configuration parameters of a GPIO pin/port.
 * @details If \ref GPIOpin_init is passed a null pointer configuration,
 *          these parameters will contain their default values.
 */
typedef struct {
  GPIO_Mode mode;   //!< Pin I/O direction      | (default GPIO_MODE_OUTPUT)
  GPIO_Type type;   //!< Pin output type        | (default GPIO_TYPE_PUSHPULL)
  GPIO_Speed speed; //!< Pin output speed       | (default GPIO_SPEED_HIGH)
  GPIO_PUPD pupd;   //!< Pin pull-up/pull-down  | (default GPIO_PUPD_NONE)
  GPIO_AF afr;      //!< Pin alternate function | (default GPIO_AF0)
} GPIO_Config;

/**
 * @brief   Struct definition for a GPIO pin.
 * @details Provides the interface for peripheral drivers and application
 *          code to configure and interact with an individual GPIO pin.
 */
typedef struct GPIOpin {
  GPIO_TypeDef *port;                                    //!< GPIO port in which the pin is located.
  GPIO_Pin pin;                                          //!< Actual location of the pin within GPIO port.
  GPIO_Config config;                                    //!< Configuration parameters for the pin.
  bool (*isSet)(struct GPIOpin *);                       //!< @see GPIOpin_isSet
  void (*set)(struct GPIOpin *);                         //!< @see GPIOpin_set
  void (*reset)(struct GPIOpin *);                       //!< @see GPIOpin_reset
  void (*toggle)(struct GPIOpin *);                      //!< @see GPIOpin_toggle
  void (*updateConfig)(struct GPIOpin *, GPIO_Config *); //!< @see GPIOpin_updateConfig
} GPIOpin_t;

GPIOpin_t GPIOpin_init(GPIO_TypeDef *, GPIO_Pin, GPIO_Config *);
bool GPIOpin_isSet(GPIOpin_t *);
void GPIOpin_set(GPIOpin_t *);
void GPIOpin_reset(GPIOpin_t *);
void GPIOpin_toggle(GPIOpin_t *);
void GPIOpin_updateConfig(GPIOpin_t *, GPIO_Config *);

/** @} */
#endif
