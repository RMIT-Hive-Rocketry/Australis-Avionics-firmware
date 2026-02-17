/***********************************************************************************
 * @file        broadcast_queue.c                                                  *
 * @brief       Simple alternative to pub/sub system for broadcasting data.        *
 *                                                                                 *
 * @{                                                                              *
 ***********************************************************************************/

#include <stdint.h>

#include "FreeRTOS.h" // IWYU pragma: keep
#include "queue.h"


/* ============================================================================================== */
/**
 * @brief Create a subscription to a broadcast.
 *
 * @param tx  Pointer to the broadcasting "station".
 * @param rx  Pointer to structure to initialize.
 **
 * ============================================================================================== */
RESULT_Broadcast_Queue_Subscribe
Broadcast_Queue_Subscribe(Broadcast_Queue_Tx_t* tx, Broadcast_Queue_Rx_t* rx) {

  // Terminate if the subscriber is already subscribed to some broadcast.
  //TODO: Double check that STM32F439ZI inits RAM to zero.
  if (rx->queue != NULL) {
    return RESULT_Broadcast_Queue_Subscribe__Member_Subscribed_To_Other;
  }

  // If tx doesn't yet point to a subscriber, then assign it.
  if (tx->head == NULL) {
    tx->head = rx;
  }
  else {

    // Check if head of list and rx are identical.
    if (tx->head == rx) {
      return RESULT_Broadcast_Queue_Subscribe__Member_Already_Exists;
    }

    // From second spot in the list, iterate until at the head (instantly termin-
    // ates for a list of one length), and check if rx already exists in the list.
    for (Broadcast_Queue_Rx_t* member = tx->head->next; member != tx->head; member = member->next) {
      if (member == rx) {
        return RESULT_Broadcast_Queue_Subscribe__Member_Already_Exists;
      }
    }

    // Initialize the queue structure.
    rx->queue = xQueueCreate(tx->length, tx->data_size);
    if (rx->queue == NULL) {
      return RESULT_Broadcast_Queue_Subscribe__xCreateQueue_Failed;
    }

    // The member does not already exist; insert after the head.
    Broadcast_Queue_Rx_t* a = tx->head;
    Broadcast_Queue_Rx_t* b = tx->head->next;
    a->next  = rx;
    rx->next = b;
  }

  // No errors have occurred in operation.
  return RESULT_Broadcast_Queue_Add__Success;

}

/* ============================================================================================== */
/**
 * @brief Broadcast some data to every queue.
 *
 * @param tx  Pointer to the broadcasting "station".
 * @param rx  Pointer to data to send.
 **
 * ============================================================================================== */
static RESULT_Broadcast_Queue_Broadcast_t
Change_Error_State(RESULT_Broadcast_Queue_Broadcast_t state, BaseType_t queue_result) {

  switch(state) {

  case RESULT_Broadcast_Queue_Broadcast__Failure_Send_Partial:
    return RESULT_Broadcast_Queue_Broadcast__Failure_Send_Partial;

  case RESULT_Broadcast_Queue_Broadcast__Success:
    if (queue_result == errQUEUE_FULL) {
      return RESULT_Broadcast_Queue_Broadcast__Failure_Send_All;
    }

  case RESULT_Broadcast_Queue_Broadcast__Failure_Send_All:
    if (queue_result == pdPASS) {
      return RESULT_Broadcast_Queue_Broadcast__Failure_Send_Partial;
    }
  }

}

RESULT_Broadcast_Queue_Broadcast_t
Broadcast_Queue_Broadcast(Broadcast_queue_Tx_t* tx, void* data) {

  RESULT_Broadcast_Queue_Broadcast_t return_state = RESULT_Broadcast_Queue_Broadcast__Success;
  BaseType_t queue_result;

  // Send to head.
  queue_result = xQueueSend(tx->head.queue, data, 0);

  // Send to rest of circular list.
  for (Broadcast_Queue_Rx_t* member = tx->head->next; member != tx->head; member = member->next) {
    xQueueSend(member.queue, data, 0);
  }

}
