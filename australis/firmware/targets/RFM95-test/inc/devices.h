/**
 * @author Matt Ricci
 */

// ALLOW FORMATTING
#ifndef DEVICES_H
#define DEVICES_H

/* ===================================================================== *
 *                           DEVICE DEFINITIONS                          *
 * ===================================================================== */

/***************************  Misc definitions  **************************/

#define LED1_PORT           GPIOC
#define LED1_PIN            0

#define LED2_PORT           GPIOA
#define LED2_PIN            1

/*********************  Device specific definitions  *********************/

#define LORA_RF_TOGGLE_PORT GPIOE                     // LoRa RF toggle on GPIOD
#define LORA_RF_TOGGLE_PIN  GPIO_PIN2                 // LoRa RF toggle on PD3
#define LORA_CS_PORT        GPIOD                     // LoRa CS on GPIOD
#define LORA_CS_PIN         GPIO_PIN3                 // LoRa CS on PD3
#define LORA_CS             LORA_CS_PORT, LORA_CS_PIN //

/**********************  Peripheral bus definitions  *********************/

#define LORA_SPI_INTERFACE  SPI3       // LoRa chip SPI bus interface on SPI3
#define LORA_SPI_PORT       GPIOC      // LoRa chip SPI bus pins on GPIOC
#define LORA_SPI_AF         GPIO_AF6   // LoRa chip SPI pin alternate function
#define LORA_SPI_SCK        GPIO_PIN10 // SCK pin on PE10
#define LORA_SPI_SDO        GPIO_PIN11 // SDI pin on PE11
#define LORA_SPI_SDI        GPIO_PIN12 // SDO pin on PE12

#endif
