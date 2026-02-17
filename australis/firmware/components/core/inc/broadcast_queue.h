/**
 * @file broadcast_queue.h
 */

// ALLOW FORMATTING
#ifndef BROADCAST_QUEUE_H
#define BROADCAST_QUEUE_H

#include <stdint.h>
#include <stddef.h>

#include "FreeRTOS.h" // IWYU pragma: keep



/* The Broadcast_Queue structures are made to form a circular list.
 * Broadcasting works by just copying data to each member of the ring.
 */

//TODO: Static compile-time structures are preferable


/** @brief Provides the context of a list of subscribers.
 */
typedef struct {
  Broadcast_Queue_Rx_t* head;
  size_t data_size;
  uint16_t length;
} Broadcast_Queue_Tx_t;

/** @brief A subscriber to a broadcast queue. The next members form a circular
 *  list of subscribers to one broadcast.
 */
typedef struct {
  QueueHandle_t queue;
  Broadcast_Queue_Rx_t* next;
} Broadcast_Queue_Rx_t;




typedef enum {
  RESULT_Broadcast_Queue_Subscribe__Success,
  RESULT_Broadcast_Queue_Subscribe__xCreateQueue_Failed,
  RESULT_Broadcast_Queue_Subscribe__Member_Already_Exists,
  RESULT_Broadcast_Queue_Subscribe__Member_Subscribed_To_Other
} RESULT_Broadcast_Queue_Subscribe ;

RESULT_Broadcast_Queue_Subscribe
Broadcast_Queue_Subscribe(Broadcast_Queue_Tx_t* tx, Broadcast_Queue_Rx_t* rx);



typedef enum {
  RESULT_Broadcast_Queue_Broadcast__Success,
  RESULT_Broadcast_Queue_Broadcast__Failure_Send_All,
  RESULT_Broadcast_Queue_Broadcast__Failure_Send_Partial
} RESULT_Broadcast_Queue_Broadcast_t;

RESULT_Broadcast_Queue_Broadcast_t
Broadcast_Queue_Broadcast(Broadcast_queue_Tx_t* tx);


#endif /* BROADCAST_QUEUE_H */
