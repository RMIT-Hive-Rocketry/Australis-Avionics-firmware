/**
 * @author Matt Ricci
 */

// ALLOW FORMATTING
#ifndef DEVICES_H
#define DEVICES_H

#include "stdbool.h"

bool initDevices();

/***************************  Misc definitions  **************************/

#define LED1_PORT         GPIOC
#define LED1_PIN          0

#define LED2_PORT         GPIOA
#define LED2_PIN          1

/*********************  Device specific definitions  *********************/

#define ACCEL_CS_PORT1    GPIOF                         // Accel 1 CS on GPIOF
#define ACCEL_CS_PIN1     GPIO_PIN12                    // Accel 1 CS on PF12
#define ACCEL_CS1         ACCEL_CS_PORT1, ACCEL_CS_PIN1 //

#define ACCEL_CS_PORT2    GPIOB                         // Accel 2 CS on GPIOB
#define ACCEL_CS_PIN2     GPIO_PIN0                     // Accel 2 CS on PB0
#define ACCEL_CS2         ACCEL_CS_PORT2, ACCEL_CS_PIN2 //

#define BARO_CS_PORT      GPIOF                         // Baro CS on GPIOF
#define BARO_CS_PIN       GPIO_PIN15                    // Baro CS on PF15
#define BARO_CS           BARO_CS_PORT, BARO_CS_PIN     //

#define GYRO_CS_PORT      GPIOF                         // Gyro CS on GPIOF
#define GYRO_CS_PIN       GPIO_PIN11                    // Gyro CS on PF11
#define GYRO_CS           GYRO_CS_PORT, GYRO_CS_PIN     //

#define FLASH_CS_PORT     GPIOA                         // Flash CS on GPIOA
#define FLASH_CS_PIN      GPIO_PIN3                     // Flash CS on PA3
#define FLASH_CS          GYRO_CS_PORT, GYRO_CS_PIN     //
#define FLASH_PAGE_SIZE   256                           // 256 bytes per page
#define FLASH_PAGE_COUNT  65536                         // 65536 total pages

/**********************  Peripheral bus definitions  *********************/

#define SUP_SPI_INTERFACE SPI1      // Sensor suite SPI bus interface on SPI1
#define SUP_SPI_PORT      GPIOA     // Sensor suite SPI bus pins on GPIOA
#define SUP_SPI_AF        GPIO_AF5  // Sensor suite SPI pin alternate function
#define SUP_SPI_SCK       GPIO_PIN5 // SCK pin on PA5
#define SUP_SPI_SDO       GPIO_PIN6 // SDI pin on PA6
#define SUP_SPI_SDI       GPIO_PIN7 // SDO pin on PA7

#endif
