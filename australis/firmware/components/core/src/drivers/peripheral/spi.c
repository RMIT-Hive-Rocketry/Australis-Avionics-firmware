/***********************************************************************************
 * @file        spi.c                                                              *
 * @author      Matt Ricci                                                         *
 * @addtogroup  SPI                                                                *
 * @brief       Implements functions for SPI device interface initialization and   *
 *              communication.                                                     *
 *                                                                                 *
 * TODO:        Rewrite initialisation to implement enum check for 8/16 bit mode   *
 * 							and assign approprriate function pointers 												 *
 * TODO:        Refactor blocking methods to pass a volatile timeout flag to allow *
 *              them to gracefully exit if the timeout is triggered.               *
 *                                                                                 *
 ***********************************************************************************/

#include "spi.h"

#include "stddef.h"

static void SPI_send8(SPI_t *, uint16_t);
static void SPI_send16(SPI_t *, uint16_t);

static void SPI_receive8(SPI_t *, volatile uint16_t *);
static void SPI_receive16(SPI_t *, volatile uint16_t *);

static void _SPI_init(SPI_TypeDef *, SPI_Config *);

/* =============================================================================== */
/**
 * @brief  Initialiser for an SPI device interface.
 *
 * @param  interface Pointer to the SPI_TypeDef struct representing the SPI interface.
 * @param  config    Pointer to SPI_Config struct for initial configuration.
 *                   This may be passed as \c NULL to initialise a default
 *                   configuration.
 *
 * @return spi       Initialised SPI_t struct.
 **
 * =============================================================================== */
SPI_t SPI_init(SPI_TypeDef *interface, SPI_Config *config) {
  // Early return error struct if peripheral is NULL
  if (interface == NULL) {
    return (SPI_t){.interface = NULL};
  }

  // Initialise SPI struct with interface
  SPI_t spi = {.interface = interface};

  // Update config and enable peripheral
  SPI_updateConfig(&spi, config);

  // Initialise remaining parameters and methods
  spi.send         = (spi.config.DFF == SPI_DFF8) ? SPI_send8 : SPI_send16;
  spi.receive      = (spi.config.DFF == SPI_DFF8) ? SPI_receive8 : SPI_receive16;
  spi.transmit     = SPI_transmit;
  spi.updateConfig = SPI_updateConfig;

  return spi;
}

// ALLOW FORMATTING
#ifndef __DOXYGEN__

/* =============================================================================== */
/**
 * @brief   Private initialiser for SPI registers.
 *
 * @param   interface Pointer to the SPI_TypeDef struct representing the SPI interface.
 * @param   config    Pointer to SPI_Config struct for initial configuration.
 *                    This may be passed as \c NULL to initialise a default
 *                    configuration. @see SPI_Config
 *
 * @return  @c NULL.
 **
 * =============================================================================== */
static void _SPI_init(SPI_TypeDef *interface, SPI_Config *config) {
  // Wait for any ongoing transactions to finish
  while (interface->SR & SPI_SR_BSY);

  // Disable peripheral and update config
  config->SPE    = false;               // Make sure SPE is disabled in config
  interface->CR1 = *(uint16_t *)config; // Update CR1 with configured values

  // Re-enable peripheral
  config->SPE    = true;                // Set SPE back to true
  interface->CR1 = *(uint16_t *)config; // Update CR1 with SPE enabled
}

#endif

/* =============================================================================== */
/**
 * @brief  Instance method to communicate a SPI transaction with slave device.
 *
 * @param  spi 			Pointer to SPI_t struct.
 * @param  data 		Data payload to be sent to slave device.
 *
 * @retval response Returns the slave device response from the transaction.
 **
 * =============================================================================== */
uint16_t SPI_transmit(SPI_t *spi, uint16_t data) {
  volatile uint16_t response;
  spi->send(spi, data);
  spi->receive(spi, &response);
  while (spi->interface->SR & SPI_SR_BSY);
  return response;
}

/* =============================================================================== */
/**
 * @brief  Send data through the SPI interface.
 *
 * @param  spi 	Pointer to SPI struct.
 * @param  data The data to send.
 *
 * @return @c NULL.
 **
 * =============================================================================== */
static void SPI_send8(SPI_t *spi, uint16_t data) {
  while (!(spi->interface->SR & SPI_SR_TXE));
  spi->interface->DR = (uint8_t)data;
}

static void SPI_send16(SPI_t *spi, uint16_t data) {
  while (!(spi->interface->SR & SPI_SR_TXE));
  spi->interface->DR = data;
}

/* =============================================================================== */
/**
 * @brief  Receive data through the SPI interface.
 *
 * @param  spi  Pointer to SPI struct.
 * @param  data Pointer to variable to receive data into.
 *
 * @return @c NULL.
 **
 * =============================================================================== */
static void SPI_receive8(SPI_t *spi, volatile uint16_t *data) {
  while (!(spi->interface->SR & SPI_SR_RXNE));
  *data = (uint8_t)spi->interface->DR;
}

/* =============================================================================== */
/**
 * @brief  Receive data through the SPI interface.
 *
 * @param  spi  Pointer to SPI struct.
 * @param  data Pointer to variable to receive data into.
 *
 * @return @c NULL.
 **
 * =============================================================================== */
static void SPI_receive16(SPI_t *spi, volatile uint16_t *data) {
  while (!(spi->interface->SR & SPI_SR_RXNE));
  *data = spi->interface->DR;
}

/* =============================================================================== */
/**
 * @brief   Update SPI peripheral configuration
 * @details Uses the provided configuration to update the SPI registers and resets the
 *          associated peripheral.
 *          As with initialisation, passing \c NULL will set the default config.
 *
 * @param   spi Pointer to SPI_t struct.
 *
 * @return  @c NULL.
 **
 * =============================================================================== */
void SPI_updateConfig(SPI_t *spi, SPI_Config *config) {
  // Initialise config with default values if passed NULL.
  if (config == NULL) {
    config = &SPI_CONFIG_DEFAULT;
  }

  // Update peripheral with new config
  spi->config = *config;

  // Initialise SPI registers and enable peripheral
  _SPI_init(spi->interface, config);
}
