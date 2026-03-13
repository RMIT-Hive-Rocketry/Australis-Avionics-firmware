/**************************************************************************************************
 * @file  canpub.c                                                                                *
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

#include "can.h"
#include "rcc.h"
#include "gpiopin.h"
#include "devices.h"
#include "broadcast_queue.h"

static TaskHandle_t vCanTransmitHandle;
static TaskHandle_t vCanReceiveHandle;




// All CAN_Queue_t types must be tracked in this array for vCanReceive
static CAN_Queue_t* can_broadcast[CAN_LISTENERS_MAX];
static uint8_t can_listener_count = 0;


Return_CAN_Queue_Create_t
CAN_Queue_Create(CAN_Queue_t* target, CAN_ID_t id) {
  
  // Create the queue data structure.
  target->id = id;
  target->queue = xQueueCreate(CAN_QUEUE_LENGTH, sizeof(CAN_Data));

  // Did queue creation fail?
  if (target->queue == NULL) {
    return Return_CAN_Queue_Create__Error_QueueCreateStatic_Failed;
  }

  // Register the queue for reception.
  if (can_listener_count < CAN_LISTENERS_MAX) {
    can_broadcast[can_listener_count++] = target;
  } else {
    return Return_CAN_Queue_Create__Error_Too_Many_Listeners;
  }

  // Everything was successful.
  return Return_CAN_Queue_Create__Success;
}

// CAN transmission queue data structures.
QueueHandle_t CAN_Transmission_Queue;

Return_CAN_Transmission_Queue_Add_t
CAN_Transmission_Queue_Add(CAN_Packet* packet) {

  // Create transmission queue on first run.
  static bool queue_init = false;
  if (!queue_init) {
    CAN_Transmission_Queue = xQueueCreate(CAN_TRANSMISSION_QUEUE_LENGTH, sizeof(CAN_Packet));
    queue_init = true;
  }

  if (xQueueSend(CAN_Transmission_Queue, packet, 0) == errQUEUE_FULL) {
    return Return_CAN_Transmission_Queue_Add__Error_Queue_Full;
  }

  return Return_CAN_Transmission_Queue_Add__Success;

}


// LoRa transceiver device
//
// TODO:
// Add deviceReady flag to driver API to indicate
// when a device struct is initialised and populated
static CAN_t *peripheral;
void CAN_setPeripheral(CAN_t peripheral_) {
  CAN_t storage = peripheral_;
  peripheral = &storage;
}

/* ============================================================================================== */
/**
 * @brief UART transmit task.
 *
 **
 * ============================================================================================== */
void vCanTransmit(void *argument) {
  const TickType_t blockTime = portMAX_DELAY;
  CAN_Packet txData;

  vCanTransmitHandle = xTaskGetCurrentTaskHandle();

  for (;;) {
    // Don't operate unless transceiver is ready
    if (peripheral == NULL) {
      continue;
    }


    // Loop through messages waiting in the mailbox.
    while (uxQueueMessagesWaiting(CAN_Transmission_Queue) > 0) {

      // Peek next packet, don't receive because we're not sure if transmission
      // will succeed.
      xQueuePeek(CAN_Transmission_Queue, &txData, 0);

      // Perform transmission
      Return_CAN_transmit_t Return_CAN_transmit = peripheral->transmit(peripheral, &txData);

      
      if (Return_CAN_transmit == Return_CAN_transmit__Success) {
        // If transmission succeeds, flush packet out of queue.
        xQueueReceive(CAN_Transmission_Queue, &txData, 0);
      } else if (Return_CAN_transmit == Return_CAN_transmit__Mailbox_Full) {
        break;
      } else {
        // A more severe error has occurred, either a timeout or a TX failure.
      }
      
    }
    
    TickType_t xLastWakeTime = xTaskGetTickCount();
    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(150));

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
  CAN_Packet rxData;

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


    Return_CAN_receive_t Return_CAN_receive;
    Return_CAN_receive = peripheral->receive(peripheral, &rxData);
    
    if (Return_CAN_receive == Return_CAN_receive__Message_Received) {

      //TODO What is the purpose of this line?
      peripheral->interface->IER |= CAN_IER_FMPIE0;
      //IER_FMPIE0: FIFO message pending interrupt enable

      // Send the message to any queues listening for the ID.
      for (uint8_t can_listener = 0; can_listener < can_listener_count; can_listener++) {
        if (can_broadcast[can_listener]->id == rxData.id) {
          xQueueSend(can_broadcast[can_listener]->queue,
                     &rxData.data, 0);
        }
      }
    }
    
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
