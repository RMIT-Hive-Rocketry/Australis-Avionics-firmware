/**************************************************************************************************
 * @file  uartcomm.c                                                                              *
 * @brief Implements the FreeRTOS tasks and Interrupt Service Routine (ISR)                       *
 *        responsible for managing uart communication.                                            *
 *                                                                                                *
 *                                                                                                *
 * @{                                                                                             *
 **************************************************************************************************/

#include "AustralisConfig.h"
#include "canpub.h"

#include "stm32f439xx.h"

#include "FreeRTOS.h"
#include "message_buffer.h"
#include "event_groups.h"
#include "portmacro.h"
#include "projdefs.h"
#include "queue.h"

#include "_topic.h"
#include "can.h"
#include "rcc.h"
#include "gpiopin.h"

// Create publication topic for UART data
//
// NOTE:
// This topic is exposed for reader
// comments in the public header.
CREATE_TOPIC(can, 10, CAN_MSG_LENGTH)
Topic *canTopic = (Topic *)&can;

static TaskHandle_t vCanTransmitHandle;
static TaskHandle_t vCanReceiveHandle;

// LoRa transceiver device
//
// TODO:
// Add deviceReady flag to driver API to indicate
// when a device struct is initialised and populated
static CAN_t *peripheral;
void CAN_setPeripheral(CAN_t *peripheral_) {
  peripheral = peripheral_;
}

void __attribute__((constructor)) init() {
  RCC_START_PERIPHERAL(APB1, CAN1);
  // CANGPIO_config();
  // CAN_Peripheral_config();

  // static CAN_t c;
  // c = CAN_init(CAN1, NULL);
  // CAN_setPeripheral(&c);
}

/* ============================================================================================== */
/**
 * @brief UART transmit task.
 *
 **
 * ============================================================================================== */
void vCanTransmit(void *argument) {
  const TickType_t blockTime = portMAX_DELAY;
  CAN_Data txData;

  vCanTransmitHandle = xTaskGetCurrentTaskHandle();

  for (;;) {
    // Don't operate unless transceiver is ready
    if (peripheral == NULL) {
      continue;
    }

    // Wait to receive message to transmit
    // BaseType_t result = WAIT_COMMENT(
    //  can.public.commentInbox, // Read from LoRa topic comment queue
    //  (void *)&txData,         // Store data in binary array
    //  portMAX_DELAY            // Block forever until comment is available
    //);

    // if (result == pdTRUE) {
    //  Transmit data if successfully retrieved from queue

    TickType_t xLastWakeTime = xTaskGetTickCount();
    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(150));

    txData.id      = 0x603;
    txData.length  = 8;
    txData.data[0] = 25;
    txData.data[1] = 59;
    CAN_transmit(peripheral, &txData);
    //}
  }
}

/* ============================================================================================== */
/**
 * @brief UART receive task.
 *
 **
 * ============================================================================================== */
void vCanReceive(void *argument) {
  const TickType_t blockTime = portMAX_DELAY;
  CAN_Data rxData;

  vCanReceiveHandle   = xTaskGetCurrentTaskHandle();

  GPIOpin_t indicator = GPIOpin_init(GPIOA, GPIO_PIN1, NULL);

  for (;;) {
    // Don't operate unless transceiver is ready
    if (peripheral == NULL) {
      continue;
    }

    // Read byte from UART Rx buffer
    xTaskNotifyWait(0, 0, NULL, portMAX_DELAY);

    indicator.toggle(&indicator);

    CAN_receive(peripheral, &rxData);

    peripheral->interface->IER |= CAN_IER_FMPIE0;

    // Publish packet data to topic
    // Topic_publish((PrivateTopic *)canTopic, (uint8_t *)&rxData);
  }
}

/* ============================================================================================== */
/**
 * @brief CAN Rx complete interrupt handler.
 *
 **
 * ============================================================================================== */
void pubCanInterrupt() {
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;

  // Exit if peripheral is not ready
  if (peripheral == NULL) {
    goto CAN_NOT_READY;
  }

  xTaskNotifyFromISR(vCanReceiveHandle, 0, eSetValueWithOverwrite, &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

  peripheral->interface->IER &= ~CAN_IER_FMPIE0;

CAN_NOT_READY:
  return;
}

/** @} */
