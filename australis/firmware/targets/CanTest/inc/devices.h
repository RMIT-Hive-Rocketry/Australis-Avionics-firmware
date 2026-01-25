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

#define LED1_PORT GPIOC
#define LED1_PIN  0

#define LED2_PORT GPIOA
#define LED2_PIN  1

/**********************  Peripheral bus definitions  *********************/

#define CAN_PORT  GPIOA      // CAN bus pins on Port A
#define CAN_AF    GPIO_AF9   //
#define CAN_TXD   GPIO_PIN12 // TXD pin on PA12
#define CAN_RXD   GPIO_PIN11 // RXD pin on PA11

#endif
