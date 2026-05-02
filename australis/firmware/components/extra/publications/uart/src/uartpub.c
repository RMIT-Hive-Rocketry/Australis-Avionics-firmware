/**************************************************************************************************
 * @file  uartcomm.c                                                                              *
 * @brief Implements the FreeRTOS tasks and Interrupt Service Routine (ISR)                       *
 *        responsible for managing uart communication.                                            *
 *                                                                                                *
 *                                                                                                *
 * @{                                                                                             *
 **************************************************************************************************/

#include "AustralisConfig.h"
#include "uartpub.h"

#include "stm32f439xx.h"

#include "FreeRTOS.h"
#include "event_groups.h"
#include "message_buffer.h"
#include "queue.h"

#include "_topic.h"
#include "uart.h"
#include "gpiopin.h"
#include "devices.h"

// Create publication topic for UART data
//
// NOTE:
// This topic is exposed for reader
// comments in the public header.
CREATE_TOPIC(uart, UART_MSG_LENGTH, sizeof(uint8_t))
Topic *uartTopic = (Topic *)&uart;

static TaskHandle_t vUartTransmitHandle;
static TaskHandle_t vUartReceiveHandle;

// LoRa transceiver device
//
// TODO:
// Add deviceReady flag to driver API to indicate
// when a device struct is initialised and populated
static UART_t *peripheral;
void UART_setPeripheral(UART_t *peripheral_) {
  peripheral = peripheral_;
}


void UART_Startup() {

  // Initialize the transmit queue
  Queue_UART_Transmit = xQueueCreate(QUEUE_UART_LENGTH, sizeof(uint8_t));

  BQueue_UART_Received = Broadcast_Queue_Create();

}


/* ============================================================================================== */
/**
 * @brief UART transmit task.
 *
 **
 * ============================================================================================== */
void vUartTransmit(void *argument) {
  const TickType_t blockTime = portMAX_DELAY;
  uint8_t txData;

  vUartTransmitHandle = xTaskGetCurrentTaskHandle();
  
  for (;;) {
    // Don't operate unless transceiver is ready
    if (peripheral == NULL) {
      continue;
    }

    // Wait to receive message to transmit
    BaseType_t result = xQueueReceive(
      Queue_UART_Transmit,      // Read from UART topic comment queue
      (void *)&txData,          // Store data in binary array
      portMAX_DELAY             // Block forever until comment is available
    );

    if (result == pdTRUE) {
      // Transmit data if successfully retrieved from queue
      peripheral->send(peripheral, txData);
    }
  }
}

/* ============================================================================================== */
/**
 * @brief UART receive task.
 *
 **
 * ============================================================================================== */
void vUartReceive(void *argument) {
  const TickType_t blockTime = portMAX_DELAY;
  uint32_t rxData;

  vUartReceiveHandle  = xTaskGetCurrentTaskHandle();

  GPIOpin_t indicator = GPIOpin_init(LED2_PORT, LED2_PIN, NULL);
  indicator.reset(&indicator);

  for (;;) {
    // Don't operate unless transceiver is ready
    if (peripheral == NULL) {
      continue;
    }

    // Read byte from UART Rx buffer
    xTaskNotifyWait(0, 0, &rxData, portMAX_DELAY);

    // Publish packet data to topic
    Broadcast_Queue_Broadcast(&BQueue_UART_Received, (uint8_t *)&rxData);
  }
}

/* ============================================================================================== */
/**
 * @brief UART Rx complete interrupt handler.
 *
 **
 * ============================================================================================== */
void pubUartInterrupt() {
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;

  // Exit if peripheral is not ready
  if (peripheral == NULL) {
    goto UART_NOT_READY;
  }

  if (peripheral->interface->SR & USART_SR_RXNE) {
    uint8_t data = peripheral->interface->DR;
    xTaskNotifyFromISR(vUartReceiveHandle, data, eSetValueWithOverwrite, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
  }

UART_NOT_READY:
  return;
}

/** @} */
