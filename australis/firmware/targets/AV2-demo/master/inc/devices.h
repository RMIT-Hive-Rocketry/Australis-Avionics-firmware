/**
 * @author Matt Ricci
 */

// ALLOW FORMATTING
#ifndef DEVICES_H
#define DEVICES_H

#include "stdbool.h"

bool initDevices();

/* ===================================================================== *
 *                           DEVICE DEFINITIONS                          *
 * ===================================================================== */

/***************************  Misc definitions  **************************/

#define LED1_PORT             GPIOC
#define LED1_PIN              0

#define LED2_PORT             GPIOA
#define LED2_PIN              1

/*********************  Device specific definitions  *********************/

#define ACCEL_CS_PORT1        GPIOA                         // Accel 1 CS on GPIOA
#define ACCEL_CS_PIN1         GPIO_PIN3                     // Accel 1 CS on PA3
#define ACCEL_CS1             ACCEL_CS_PORT1, ACCEL_CS_PIN1 //

#define ACCEL_CS_PORT2        GPIOB                         // Accel 2 CS on GPIOB
#define ACCEL_CS_PIN2         GPIO_PIN1                     // Accel 2 CS on PB1
#define ACCEL_CS2             ACCEL_CS_PORT2, ACCEL_CS_PIN2 //

#define BARO_CS_PORT          GPIOA                         // Baro CS on GPIOA
#define BARO_CS_PIN           GPIO_PIN2                     // Baro CS on PA2
#define BARO_CS               BARO_CS_PORT, BARO_CS_PIN     //

#define GYRO_CS_PORT          GPIOF                         // Gyro CS on GPIOF
#define GYRO_CS_PIN           GPIO_PIN11                    // Gyro CS on PF11
#define GYRO_CS               GYRO_CS_PORT, GYRO_CS_PIN     //

#define FLASH_CS_PORT         GPIOE                         // Flash CS on GPIOE
#define FLASH_CS_PIN          GPIO_PIN11                    // Flash CS on PE11
#define FLASH_CS              GYRO_CS_PORT, GYRO_CS_PIN     //
#define FLASH_PAGE_SIZE       256                           // 256 bytes per page
#define FLASH_PAGE_COUNT      65536                         // 65536 total pages

#define LORA_RF_TOGGLE_PORT   GPIOE                         // LoRa RF toggle on GPIOD
#define LORA_RF_TOGGLE_PIN    GPIO_PIN2                     // LoRa RF toggle on PD3
#define LORA_CS_PORT          GPIOD                         // LoRa CS on GPIOD
#define LORA_CS_PIN           GPIO_PIN3                     // LoRa CS on PD3
#define LORA_CS               LORA_CS_PORT, LORA_CS_PIN     //
#define LORA_BW               SX1272_BW500                  // 500KHz bandwidth
#define LORA_SF               SX1272_SF9                    // Spreading factor 9
#define LORA_CR               SX1272_CR5                    // Coding rate 4/5

#define USB_INTERFACE         USART1                        // USB on UART1
#define USB_PORT              GPIOA                         // UART pins on GPIOA
#define USB_TX_PIN            GPIO_PIN9                     // USB Tx on PA9
#define USB_RX_PIN            GPIO_PIN10                    // USB Rx on PA10
#define USB_BAUD              921600                        // 921600bps baud rate
#define USB_OVERSAMPLE        OVER8                         // 8-bit oversampling

#define GPS_INTERFACE         USART3                        // GPS on UART3
#define GPS_PORT              GPIOD                         // UART pins on GPIOD
#define GPS_TX_PIN            GPIO_PIN8                     // GPS Tx on PD8
#define GPS_RX_PIN            GPIO_PIN9                     // GPS Rx on PD9
#define GPS_RESET_PORT        GPIOD                         // GPS reset pin on GPIOD
#define GPS_RESET_PIN         GPIO_PIN13                    // Reset pin PD13
#define GPS_RESET             GPS_RESET_PORT, GPS_RESET_PIN //
#define GPS_BAUD              115200                        // 115200bps baud rate

/**********************  Peripheral bus definitions  *********************/

#define SENSORS_SPI_INTERFACE SPI1       // Sensor suite SPI bus interface on SPI1
#define SENSORS_SPI_PORT      GPIOA      // Sensor suite SPI bus pins on GPIOA
#define SENSORS_SPI_AF        GPIO_AF5   // Sensor suite SPI pin alternate function
#define SENSORS_SPI_SCK       GPIO_PIN5  // SCK pin on PA5
#define SENSORS_SPI_SDO       GPIO_PIN6  // SDI pin on PA6
#define SENSORS_SPI_SDI       GPIO_PIN7  // SDO pin on PA7

#define FLASH_SPI_INTERFACE   SPI4       // Flash chip SPI bus interface on SPI4
#define FLASH_SPI_PORT        GPIOE      // Flash chip SPI bus pins on GPIOE
#define FLASH_SPI_AF          GPIO_AF5   // Flash chip SPI pin alternate function
#define FLASH_SPI_SCK         GPIO_PIN12 // SCK pin on PE12
#define FLASH_SPI_SDO         GPIO_PIN13 // SDI pin on PE13
#define FLASH_SPI_SDI         GPIO_PIN14 // SDO pin on PE14

#define LORA_SPI_INTERFACE    SPI3       // LoRa chip SPI bus interface on SPI3
#define LORA_SPI_PORT         GPIOC      // LoRa chip SPI bus pins on GPIOC
#define LORA_SPI_AF           GPIO_AF6   // LoRa chip SPI pin alternate function
#define LORA_SPI_SCK          GPIO_PIN10 // SCK pin on PE10
#define LORA_SPI_SDO          GPIO_PIN11 // SDI pin on PE11
#define LORA_SPI_SDI          GPIO_PIN12 // SDO pin on PE12

#endif
