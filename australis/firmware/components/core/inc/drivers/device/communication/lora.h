/**
 * @file lora.h
 * @brief Defines the API for LoRa communication.
 * @addtogroup API_lora
 * @{
 */

// ALLOW FORMATTING
#ifndef LORA_H
#define LORA_H

#include "stdint.h"
#include "stdbool.h"

/**
 * @ingroup API_Communication
 * @addtogroup API_LoRa LoRa
 * @{
 */

/**
 * @brief   LoRa base operating mode enum
 */
typedef enum {
  LORA_MODE_TX,   // Transmission mode
  LORA_MODE_RX,   // Continuous receive mode
  LORA_MODE_OTHER // Driver defined operating mode
} LoRa_Mode;

typedef struct LoRa {
  LoRa_Mode currentMode;                                     //!< Current operating mode.
  void (*transmit)(struct LoRa *, uint8_t *, uint8_t);       //!< LoRa transmit method.
  void (*startReceive)(struct LoRa *);                       //!< LoRa continuous receive method.
  uint8_t (*readReceive)(struct LoRa *, uint8_t *, uint8_t); //!< LoRa receive buffer read method.
  void (*clearIRQ)(struct LoRa *, uint8_t);                  //!< LoRa IRQ clear method.
} LoRa_t;

/** @} */
#endif
