/***********************************************************************************
 * @file        uart.c                                                             *
 * @author      Matt Ricci                                                         *
 * @addtogroup  UART                                                               *
 * @brief       Brief description of the file's purpose.                           *
 *                                                                                 *
 * @todo Implement printf                                                          *
 ***********************************************************************************/

#include "uart.h"
#include "stddef.h"
#include "config.h"

static void _UART_init(UART_t *uart, uint32_t baud, UART_Config *config);

/* =============================================================================== */
/**
 * @brief Initialiser for a UART device interface.
 *
 * @param  *interface  Pointer to UART interface struct.
 * @param  baud 			 UART baud rate.
 * @param  *config 		 Pointer to UART configuration struct.
 *
 * @return Initialised @c UART_t driver struct.
 **
 * =============================================================================== */
UART_t UART_init(USART_TypeDef *interface, uint32_t baud, UART_Config *config) {
  // Early return error struct if peripheral is NULL
  if (interface == NULL) {
    return (UART_t){.interface = NULL};
  }

  // Initialise uart struct with interface
  UART_t uart    = {.interface = interface};

  uart.setBaud   = UART_setBaud;
  uart.send      = UART_send;
  uart.sendBytes = UART_sendBytes;
  uart.print     = UART_print;
  uart.println   = UART_println;
  uart.receive   = UART_receive;
  uart.interface = interface;
  uart.baud      = baud;

  // Update config and enable peripheral
  UART_updateConfig(&uart, config);

  return uart;
}

// ALLOW FORMATTING
#ifndef __DOXYGEN__

/* =============================================================================== */
/**
 * @brief   Private initialiser for UART registers.
 *
 * @param   interface Pointer to the USART_TypeDef struct representing the UART
 *                    interface.
 * @param   config    Pointer to UART_Config struct for initial configuration.
 *                    This may be passed as \c NULL to initialise a default
 *                    configuration. @see UART_Config
 *
 * @return  @c NULL.
 **
 * =============================================================================== */
static void _UART_init(UART_t *uart, uint32_t baud, UART_Config *config) {
  USART_TypeDef *interface = uart->interface;

  // Disable peripheral and update config
  config->UE      = false;                                    // Make sure UE is disabled in config
  interface->CR1 &= *(uint16_t *)config & ~UART_CR1_RESERVED; // Update CR1 with configured values
  interface->CR1 |= *(uint16_t *)config & ~UART_CR1_RESERVED; // Update CR1 with configured values

  // Update baud rate
  UART_setBaud(uart, baud);
}

#endif

/********************************** INTERFACE METHODS ********************************/

void UART_setBaud(UART_t *uart, uint32_t baud) {
  USART_TypeDef *interface = uart->interface;

  // Wait for interface to be freed
  while ((interface->SR & USART_SR_TXE) == 0 && !(interface->SR & USART_SR_TC));
  interface->CR1 &= ~USART_CR1_UE;
  while ((interface->SR & USART_SR_RXNE) == 1) {
    (void)(interface->DR & 0xFF);
  }

  // Calculate relevant APBx clock for peripheral
  uint32_t sysclk = HSE_USED ? SYSCLK_HSE : SYSCLK_HSI;
  uint8_t hpre    = AHBPresc[((RCC->CFGR & RCC_CFGR_HPRE) >> RCC_CFGR_HPRE_Pos)];

  uint8_t ppre;
  // Set prescale value according to peripheral bus
  if (interface == USART1 || interface == USART6) {
    ppre = APBPresc[((RCC->CFGR & RCC_CFGR_PPRE2) >> RCC_CFGR_PPRE2_Pos)];
  } else {
    ppre = APBPresc[((RCC->CFGR & RCC_CFGR_PPRE1) >> RCC_CFGR_PPRE1_Pos)];
  }

  // Calculate peripheral bus clock from prescalers
  uint32_t pclk     = sysclk / hpre / ppre;

  uint16_t usartDiv = ((pclk / baud) * (1 + uart->config.OVER8));
  if (uart->config.OVER8 && (usartDiv & 0x08)) {
    usartDiv &= ~0x0F;
    usartDiv |= 0x07;
  }
  interface->BRR &= 0xFFFF0000; // Clear mantissa and div in baud rate reg
  interface->BRR |= usartDiv;   // Set baud rate

  interface->CR1 |= USART_CR1_UE;
}

/* =============================================================================== */
/**
 * @brief Sends a single byte of data over the UART interface.
 *
 * @param *uart  Pointer to UART struct.
 * @param data   Byte of data to be sent.
 * @return @c NULL.
 **
 * =============================================================================== */
void UART_send(UART_t *uart, uint8_t data) {
  USART_TypeDef *interface = uart->interface;
  while ((interface->SR & USART_SR_TXE) == 0);
  interface->DR = data;
  while ((interface->SR & USART_SR_TC) == 0);
}

/* =============================================================================== */
/**
 * @brief Sends an array of bytes over the UART interface.
 *
 * @param *uart   Pointer to UART struct.
 * @param *data   Pointer to the array of bytes to be sent.
 * @param length  Number of bytes to send.
 * @return @c NULL.
 **
 * =============================================================================== */
void UART_sendBytes(UART_t *uart, uint8_t *data, int length) {
  for (int i = 0; i < length; i++) {
    UART_send(uart, data[i]);
  }
}

/* =============================================================================== */
/**
 * @brief Sends a null terminated string over the UART interface.
 *
 * @param *uart  Pointer to UART struct.
 * @param *data  Pointer to the string of characters to be sent.
 * @return @c NULL.
 **
 * =============================================================================== */

void UART_print(UART_t *uart, char *data) {
  int i = 0;
  while (data[i] != '\0') {
    UART_send(uart, data[i++]);
  }
}

/* =============================================================================== */
/**
 * @brief Sends a null terminated string over the UART interface. Terminates with
 *        a line feed control character `\n`.
 *
 * @param *uart  Pointer to UART struct.
 * @param *data  Pointer to the string of characters to be sent.
 * @return @c NULL.
 **
 * =============================================================================== */

void UART_println(UART_t *uart, char *data) {
  UART_print(uart, data);
  UART_print(uart, "\r\n");
}

/* =============================================================================== */
/**
 * @brief Receives a single byte of data from the UART interface.
 *
 * @param *uart  Pointer to UART struct.
 * @return       The received byte of data.
 **
 * =============================================================================== */
bool UART_receive(UART_t *uart, uint8_t* data) {
  if (uart->buffer_head != uart->buffer_tail) {
    *data = uart->buffer[uart->buffer_head++];
    return true;
  } else {
    return false;
  }
}

/* =============================================================================== */
/**
 * @brief   Update UART peripheral configuration
 * @details Uses the provided configuration to update the UART registers and resets the
 *          associated peripheral.
 *          As with initialisation, passing \c NULL will set the default config.
 *
 * @param   uart Pointer to UART_t struct.
 *
 * @return  @c NULL.
 **
 * =============================================================================== */
void UART_updateConfig(UART_t *uart, UART_Config *config) {
  // Initialise config with default values if passed NULL.
  if (config == NULL) {
    config = &UART_CONFIG_DEFAULT;
  }

  // Update peripheral with new config
  uart->config = *config;

  // Initialise SPI registers and enable peripheral
  _UART_init(uart, uart->baud, config);
}


// This function should be called by all USART IRQ functions.
void USART_Generic_IRQHandler(UART_t* uart) {

  if (uart->interface->SR & USART_SR_RXNE) {

    // TODO: Handle alternative word sizes and parity.
    uint8_t data = (uint8_t) (uart->interface->DR & 0xFF);

    // Grab from buffer, then clear IRQ.
    uart->buffer[uart->buffer_tail++] = data;
    uart->interface->SR &= ~USART_SR_RXNE;
  }
  //else if (uart->interface->SR & USART_SR_TXNE) {
  //TODO: Don't need this case yet, but in future, add a tx buffer.
  //}

}
