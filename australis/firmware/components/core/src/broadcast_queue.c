/***********************************************************************************
 * @file        broadcast_queue.c                                                  *
 * @brief       Simple alternative to pub/sub system for broadcasting data.        *
 *                                                                                 *
 * @{                                                                              *
 ***********************************************************************************/

#include <stdint.h>

#include "FreeRTOS.h" // IWYU pragma: keep
#include "queue.h"

#include "broadcast_queue.h"



Broadcast_Queue_t
Broadcast_Queue_Create(size_t data_size, uint16_t length) {
  Broadcast_Queue_t bqueue;
  bqueue.data_size = data_size;
  bqueue.length    = length;
  return bqueue;
}

/* ============================================================================================== */
/**
 * @brief Create a subscription to a broadcast.
 *
 * @param broadcast  Pointer to the broadcasting "station".
 * @param subscriber  Pointer to structure to initialize.
 **
 * ============================================================================================== */
RESULT_Broadcast_Queue_Subscribe
Broadcast_Queue_Subscribe(Broadcast_Queue_t* broadcast, Broadcast_Queue_Member_t* subscriber) {

  // Terminate if the subscriber is already subscribed to some broadcast.
  //TODO: Double check that STM32F439ZI inits RAM to zero.
  if (subscriber->queue != NULL) {
    return RESULT_Broadcast_Queue_Subscribe__Member_Subscribed_To_Other;
  }

  // If broadcast doesn't yet point to a subscriber, then assign it.
  if (broadcast->head == NULL) {
    broadcast->head = subscriber;
  }
  else {

    // Check if head of list and subscriber are identical.
    if (broadcast->head == subscriber) {
      return RESULT_Broadcast_Queue_Subscribe__Member_Already_Exists;
    }

    // From second spot in the list, iterate until at the head (instantly termin-
    // ates for a list of one length), and check if subscriber already exists in the list.
    for (Broadcast_Queue_Member_t* member = broadcast->head->next; member != broadcast->head; member = member->next) {
      if (member == subscriber) {
        return RESULT_Broadcast_Queue_Subscribe__Member_Already_Exists;
      }
    }

    // Initialize the queue structure.
    subscriber->queue = xQueueCreate(broadcast->length, broadcast->data_size);
    if (subscriber->queue == NULL) {
      return RESULT_Broadcast_Queue_Subscribe__xCreateQueue_Failed;
    }

    // The member does not already exist; insert after the head.
    Broadcast_Queue_Member_t* a = broadcast->head;
    Broadcast_Queue_Member_t* b = broadcast->head->next;
    a->next  = subscriber;
    subscriber->next = b;
  }

  // No errors have occurred in operation.
  return RESULT_Broadcast_Queue_Broadcast__Success;

}

/* ============================================================================================== */
/**
 * @brief Broadcast some data to every queue.
 *
 * @param broadcast  Pointer to the broadcasting "station".
 * @param data       Pointer to data to send.
 **
 * ============================================================================================== */


RESULT_Broadcast_Queue_Broadcast_t
Broadcast_Queue_Broadcast(Broadcast_Queue_t* broadcast, void* data) {

  // Check broadcast is initialized.
  if (broadcast == NULL) {
    return RESULT_Broadcast_Queue_Broadcast__Broadcast_Null;
  }

  // Check if there are subscribers.
  if (broadcast->head == NULL) {
    return RESULT_Broadcast_Queue_Broadcast__Subscribers_None;
  }


  // Perform broadcast.

  RESULT_Broadcast_Queue_Broadcast_t return_state = RESULT_Broadcast_Queue_Broadcast__Success;
  BaseType_t queue_result;

  uint16_t queue_pass = 0;
  uint16_t queue_fail = 0;

  // Send to head.
  queue_result = xQueueSend(broadcast->head->queue, data, 0);

  switch(queue_result)
    {
    case pdPASS:        queue_pass++; break;
    case errQUEUE_FULL: queue_fail++; break;
    }

  // Send to rest of circular list.
  for (Broadcast_Queue_Member_t* member = broadcast->head->next; member != broadcast->head; member = member->next) {

    xQueueSend(member->queue, data, 0);

    switch(queue_result)
      {
      case pdPASS:        queue_pass++; break;
      case errQUEUE_FULL: queue_fail++; break;
      }

  }

  // Return the success rate of filling the queues.

  if ((queue_pass > 0) && (queue_fail > 0)) {
    return RESULT_Broadcast_Queue_Broadcast__Failure_Send_Partial;
  }
  else if ((queue_pass == 0) && (queue_fail > 0)) {
    return RESULT_Broadcast_Queue_Broadcast__Failure_Send_All;
  }
  else if ((queue_pass > 0) && (queue_fail == 0)) {
    return RESULT_Broadcast_Queue_Broadcast__Success;
  }

  return RESULT_Broadcast_Queue_Broadcast__Unknown_Result;

}
