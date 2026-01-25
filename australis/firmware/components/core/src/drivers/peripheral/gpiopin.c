/***********************************************************************************
 * @file        gpiopin.c                                                          *
 * @author      Matt Ricci                                                         *
 * @addtogroup  GPIO_Pin GPIO Pin                                                  *
 * @brief       Implements functions for GPIO peripheral pin interface.            *
 *              Includes methods to initialise, set, reset and update the config   *
 *              of an individual GPIO pin.                                         *
 ***********************************************************************************/

#include "gpiopin.h"
#include "stddef.h"

static void _GPIOpin_init(GPIO_TypeDef *, GPIO_Pin, GPIO_Config *);

/* =============================================================================== */
/**
 * @brief  Initialiser for a GPIO peripheral pin interface.
 *
 * @param  port   Pointer to the GPIO_TypeDef struct representing the pin's port.
 * @param  pin    Enum quantified value of the pin's position in its port.
 * @param  config Pointer to GPIO_Config struct for initial configuration.
 *                This may be passed as \c NULL to initialise a default
 *                configuration.
 *
 * @return Initialised GPIOpin_t struct.
 **
 * =============================================================================== */
GPIOpin_t GPIOpin_init(GPIO_TypeDef *port, GPIO_Pin pin, GPIO_Config *config) {
  // Early return error struct if port is NULL
  if (port == NULL) {
    return (GPIOpin_t){.port = NULL};
  }

  // Create GPIO struct from parameters and initialise methods
  GPIOpin_t gpio;
  gpio.port         = port;
  gpio.pin          = pin;
  gpio.isSet        = GPIOpin_isSet;
  gpio.set          = GPIOpin_set;
  gpio.reset        = GPIOpin_reset;
  gpio.toggle       = GPIOpin_toggle;
  gpio.updateConfig = GPIOpin_updateConfig;

  // Update config and enable peripheral
  GPIOpin_updateConfig(&gpio, config);

  // Return the new GPIO struct
  return gpio;
}

// ALLOW FORMATTING
#ifndef __DOXYGEN__

/* =============================================================================== */
/**
 * @brief   Private initialiser for GPIO registers.
 * @details Enables and resets the GPIO port in RCC and sets configuration registers.
 *
 * @param   port   Pointer to the GPIO_TypeDef struct representing the pin's port.
 * @param   pin    Enum quantified value of the pin's position in its port.
 * @param   config Pointer to GPIO_Config struct for initial configuration.
 *
 * @return  @c NULL.
 **
 * =============================================================================== */
static void _GPIOpin_init(GPIO_TypeDef *port, GPIO_Pin pin, GPIO_Config *config) {

  // Get index of supplied port by subtracting GPIOA address and dividing by size
  int portIndex           = ((uint32_t)port - GPIOA_BASE) / GPIO_PERIPHERAL_SIZE;

  port->MODER            &= ~(0b11 << (2 * pin));                       // Clear MODER bits for pin
  port->MODER            |= (config->mode << (2 * pin));                // Shift in mode bits from config

  port->OTYPER           &= ~(0b01 << pin);                             // Clear OTYPE bits for pin
  port->OTYPER           |= (config->type << pin);                      // Shift in type bits from config

  port->OSPEEDR          &= ~(0b11 << (2 * pin));                       // Clear OSPEEDR bits for pin
  port->OSPEEDR          |= (config->speed << (2 * pin));               // Shift in speed bits from config

  port->PUPDR            &= ~(0b11 << (2 * pin));                       // Clear PUPDR bits for pin
  port->PUPDR            |= (config->pupd << (2 * pin));                // Shift in pupd bits from config

  volatile uint32_t *afr  = (pin <= 7) ? &port->AFR[0] : &port->AFR[1]; // Select AFRL (pin<=7) or AFRH (pin>7)
  *afr                   &= ~(0b1111 << (4 * (pin & 7)));               // Clear AFR bits for pin
  *afr                   |= (config->afr << (4 * (pin & 7)));           // Shift in afr bits from config
}

#endif

/* =============================================================================== */
/**
 * @brief
 * @details
 *
 * @param
 *
 * @return
 **
 * =============================================================================== */
bool GPIOpin_isSet(GPIOpin_t *gpio) {
  return (gpio->port->IDR & (0b01 << gpio->pin));
}

/* =============================================================================== */
/**
 * @brief   Set the selected GPIO pin
 * @details Uses a logic OR to set the pin's current value in the data register.
 *
 * @param   gpio Pointer to GPIOpin_t struct.
 *
 * @return  @c NULL.
 **
 * =============================================================================== */
void GPIOpin_set(GPIOpin_t *gpio) {
  gpio->port->ODR |= (0b01 << gpio->pin);
}

/* =============================================================================== */
/**
 * @brief   Clear the selected GPIO pin
 * @details Uses a logic AND to clear the pin's current value in the data register.
 *
 * @param   gpio Pointer to GPIOpin_t struct.
 *
 * @return  @c NULL.
 **
 * =============================================================================== */
void GPIOpin_reset(GPIOpin_t *gpio) {
  gpio->port->ODR &= ~(0b01 << gpio->pin);
}

/* =============================================================================== */
/**
 * @brief   Toggle the selected GPIO pin
 * @details Uses a logic XOR to invert the pin's current value in the data register.
 *
 * @param   gpio Pointer to GPIOpin_t struct.
 *
 * @return  @c NULL.
 **
 * =============================================================================== */
void GPIOpin_toggle(GPIOpin_t *gpio) {
  gpio->port->ODR ^= (0b01 << gpio->pin);
}

/* =============================================================================== */
/**
 * @brief   Update GPIO pin configuration
 * @details Uses the provided configuration to update the GPIO registers and resets the
 *          associated port in the RCC.
 *          As with initialisation, passing \c NULL will set the default config.
 *
 * @param   gpio Pointer to GPIOpin_t struct.
 *
 * @return  @c NULL.
 **
 * =============================================================================== */
void GPIOpin_updateConfig(GPIOpin_t *gpio, GPIO_Config *config) {
  // Initialise config with default values if passed NULL.
  if (config == NULL) {
    config = &GPIO_CONFIG_DEFAULT;
  }

  // Update peripheral with new config
  gpio->config = *config;

  // Initialise GPIO registers and enable peripheral
  _GPIOpin_init(gpio->port, gpio->pin, config);
}
