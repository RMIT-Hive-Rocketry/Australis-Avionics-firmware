/**************************************************************************************************
 * @file  loracomm.c                                                                              *
 * @brief Implements the FreeRTOS tasks and Interrupt Service Routine (ISR)                       *
 *        responsible for managing LoRa communication.                                            *
 *                                                                                                *
 * This file contains the implementation for LoRa transmit and receive tasks. It utilizes a       *
 * publication topic (`loraTopic`) to interface with other parts of the application.              *
 *                                                                                                *
 * The transmit task waits for messages on the topic's queue and transmits them via the LoRa      *
 * transceiver                                                                                    *
 *                                                                                                *
 * The receive task waits for notifications from the ISR (indicating a received packet), reads    *
 * the data from the transceiver, and publishes it to the topic.                                  *
 *                                                                                                *
 * The `EXTI1_IRQHandler` handles interrupts from the LoRa module (e.g., Tx Done, Rx Done),       *
 * notifying the appropriate task to proceed.                                                     *
 *                                                                                                *
 * @{                                                                                             *
 **************************************************************************************************/

#include <string.h>

#include "FreeRTOS.h" // IWYU pragma: keep
#include "portmacro.h"
#include "projdefs.h"
#include "event_groups.h"
#include "message_buffer.h"
#include "queue.h"

#include "interrupts/interrupts.h"

#include "AustralisConfig.h"
#include "AustralisCore.h"
#include "rtos/_messageacquisition.h"
#include "gpiopin.h"
#include "lora.h"
#include "topic.h"
#include "_topic.h"
#include "broadcast_queue.h"

#include "lorapub.h"

static TaskHandle_t vLoRaTransmitHandle;
static TaskHandle_t vLoRaReceiveHandle;


QueueHandle_t Queue_LoRa_Transmit;
Broadcast_Queue_t BQueue_LoRa_Received;

static bool PubLora_handleComment(TopicHandle_t topic, void *data, size_t size);
static bool PubLora_acquireData(TopicHandle_t topic);

/* =============================================================================== */
/**
 * @brief
 *
 **
 * =============================================================================== */
bool PubLora_startup(TopicHandle_t topic, void *context) {

  Queue_LoRa_Transmit = xQueueCreate(QUEUE_LORA_LENGTH, sizeof(LoRa_Message_t));

  if (Queue_LoRa_Transmit == NULL) {
    return false;
  }

  BQueue_LoRa_Received = Broadcast_Queue_Create(sizeof(LoRa_Message_t), 8); //TODO magic number

  return true;
}


/* =============================================================================== */
/**
 * @brief
 *
 **
 * =============================================================================== */
void PubLora_handleInterrupt(InterruptContext *interruptContext) {
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;

  TopicHandle_t topic                 = interruptContext->context;
  LoRa_t *transceiver                 = (LoRa_t *)topic->context;

  TaskHandle_t activeHandle           = NULL;

  if (transceiver->currentMode == LORA_MODE_TX) {
    // Finished transmitting, return to receive mode
    transceiver->startReceive(transceiver);
  } else if (transceiver->currentMode == LORA_MODE_RX) {
    queueAcquisition(topic);
  }

  // Notify active task for unblock
  xTaskNotifyIndexedFromISR(activeHandle, 1, 0, eNoAction, &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/* =============================================================================== */
/**
 * @brief
 *
 **
 * =============================================================================== */
static bool PubLora_handleComment(TopicHandle_t topic, void *data, size_t size) {

  PubLora_Context *ctx = topic->context;

  // Context must not point to NULL...
  ASSERT(ctx);

  LoRa_t *transceiver  = ctx->transceiver;
  GPIOpin_t *rfToggle  = ctx->rfToggle;
  volatile bool *ready = ctx->ready;

  // Transceiver handle must not be NULL...
  ASSERT(transceiver);
  ASSERT(ready);

  if (ctx->rfToggle) {
    // Toggle RF front-end for transmit
    rfToggle->reset(rfToggle);
  }

  transceiver->transmit(transceiver, data, size);

  while (!*ready);

  if (rfToggle) {
    // Toggle RF front-end for receive
    rfToggle->set(rfToggle);
  }

  // Continue receiving
  transceiver->startReceive(transceiver);
  return false;
}

/* =============================================================================== */
/**
 * @brief
 *
 **
 * =============================================================================== */
static bool PubLora_acquireData(TopicHandle_t topic) {

  PubLora_Context *ctx = topic->context;

  // Context must not point to NULL...
  ASSERT(ctx);

  LoRa_t *transceiver  = ctx->transceiver;
  volatile bool *ready = ctx->ready;

  // Transceiver handle must not be NULL...
  ASSERT(transceiver);
  ASSERT(ready);

  // TODO:
  // Need to add method to transceiver to read number of bytes waiting in next packet.
  // This function will be called by the message acquisition task, so we can use its stack
  // to receive the packet and then send the data to dispatch.
  //
  // Read received packet from transceiver
  // rxData.length = transceiver->readReceive(transceiver, rxData.data, LORA_MSG_LENGTH);

  // TODO: Send data to dispatch (i.e. call queueArticle() with received data)

  return false;
}


// LoRa transceiver device
//
// TODO:
// Add deviceReady flag to driver API to indicate
// when a device struct is initialised and populated
static LoRa_t *transceiver;
void loraPub_setTransceiver(LoRa_t *transceiver_) {
  transceiver = transceiver_;
}
LoRa_t *loraPub_getTransceiver() { return transceiver; }

// Optional switch for RF frontend
static GPIOpin_t *rfToggle;
void loraPub_setRfToggle(GPIOpin_t *rfToggle_) {
  rfToggle = rfToggle_;
}

/* ============================================================================================== */
/**
 * @brief LoRa transmit task.
 *
 * Handles transmission of data to the LoRa transceiver.
 *
 * This task blocks until a comment is received on the LoRa topic. Once received, the data is
 * transmitted and the task blocks until notified of transmit completion.
 **
 * ============================================================================================== */
void vLoRaTransmit(void *argument) {

  const TickType_t blockTime = portMAX_DELAY;

  vLoRaTransmitHandle = xTaskGetCurrentTaskHandle();

  LoRa_Message_t txData;

  for (;;) {
    // Don't operate unless transceiver is ready
    if (transceiver == NULL)
      continue;

    // Wait to receive message to transmit
    xQueueReceive(Queue_LoRa_Transmit, &txData, portMAX_DELAY);

    // Transmit data if successfully retrieved from queue
    if (result == pdTRUE) {

      if (rfToggle)
        // Toggle RF front-end for transmit
        rfToggle->reset(rfToggle);

      // Send data to transmit
      transceiver->transmit(transceiver, txData.data, txData.length);

      // Wait for notification from ISR
      xTaskNotifyWaitIndexed(1, 0, 0, NULL, blockTime);

      if (rfToggle)
        // Toggle RF front-end for receive
        rfToggle->set(rfToggle);

      // Continue receiving
      transceiver->startReceive(transceiver);
    }
  }
}

/* ============================================================================================== */
/**
 * @brief LoRa receive task.
 *
 * Handles receiving data from the LoRa transceiver.
 *
 * This task blocks until a notification is sent of a received packet. This can only occur after
 * being set to receive mode, either by the transmit task or through a manual override in the
 * driver. Once notified, the packet data is read from the device and published to the LoRa topic.
 **
 * ============================================================================================== */
void vLoRaReceive(void *argument) {

  const TickType_t blockTime = portMAX_DELAY;

  vLoRaReceiveHandle = xTaskGetCurrentTaskHandle();

  LoRa_Message_t rxData;

  for (;;) {
    // Don't operate unless transceiver is ready
    if (transceiver == NULL)
      continue;

    // Wait for notification from ISR
    xTaskNotifyWaitIndexed(1, 0, 0, NULL, blockTime);

    // Read received packet from transceiver
    rxData.length = transceiver->readReceive(transceiver, rxData.data, LORA_MSG_LENGTH);

    // Publish packet data to topic
    Broadcast_Queue_Broadcast(&BQueue_LoRa_Received, rxData);
  }
}

/* ============================================================================================== */
/**
 * @brief LoRa Tx/Rx complete interrupt handler.
 *
 * Handles the external interrupt triggered by the Tx and Rx complete signals from the LoRa
 * transceiver. Upon interrupt, the appropriate task is notified according to the current LoRa
 * operating mode.
 **
 * ============================================================================================== */
void loraPub_interrupt(void) {
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;

  // Exit if transceiver is not ready
  if (transceiver == NULL)
    goto LORA_NOT_READY;

  TaskHandle_t activeHandle = NULL;

  // Assign currently task handle for operation
  if (transceiver->currentMode == LORA_MODE_TX) {
    activeHandle = vLoRaTransmitHandle;
  } else if (transceiver->currentMode == LORA_MODE_RX) {
    activeHandle = vLoRaReceiveHandle;
  }

  // Exit if not initialised
  if (activeHandle == NULL)
    goto LORA_NOT_READY;

  // Notify active task for unblock
  xTaskNotifyIndexedFromISR(activeHandle, 1, 0, eNoAction, &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

 LORA_NOT_READY:
  // Clean-up and exit
  // TODO: Implement specific clear methods for
  //       Tx and Rx IRQs for better abstraction
  transceiver->clearIRQ(transceiver, 0xFF);
}

/** @} */
