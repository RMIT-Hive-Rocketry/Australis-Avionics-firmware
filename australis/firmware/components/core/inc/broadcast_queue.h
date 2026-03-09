/**
 * @file broadcast_queue.h
 */

// ALLOW FORMATTING
#ifndef BROADCAST_QUEUE_H
#define BROADCAST_QUEUE_H

#include <stdint.h>
#include <stddef.h>

#include "FreeRTOS.h" // IWYU pragma: keep
#include "queue.h"


/* The Broadcast_Queue structures are made to form a circular list.
 * Broadcasting works by just copying data to each member of the ring.
 */

//TODO: Static compile-time structures are preferable


/** @brief A subscriber to a broadcast queue. The next members form a circular
 *  list of subscribers to one broadcast.
 */
typedef struct Broadcast_Queue_Member {
  QueueHandle_t queue;            //!< Queue structure.
  struct Broadcast_Queue_Member* next; //!< Following member in circular list.
} Broadcast_Queue_Member_t;

/** @brief Provides the context of a list of subscribers.
 */
typedef struct {
  Broadcast_Queue_Member_t* head; //!< Head of circular list.
  size_t data_size;             //!< Data size of all lists.
  uint16_t length;              //!< Length of all lists.
} Broadcast_Queue_t;

Broadcast_Queue_t
Broadcast_Queue_Create(size_t data_size, uint16_t length);

Broadcast_Queue_Member_t
Broadcast_Queue_Member_Create();

/** @brief Create a subscription to a broadcast. Requires a defined but
 *  uninitialized Broadcast_Queue_Member_t variable.
 */
typedef enum {
  RESULT_Broadcast_Queue_Subscribe__Success,
  RESULT_Broadcast_Queue_Subscribe__xCreateQueue_Failed,
  RESULT_Broadcast_Queue_Subscribe__Member_Already_Exists,
  RESULT_Broadcast_Queue_Subscribe__Member_Subscribed_To_Other
} RESULT_Broadcast_Queue_Subscribe ;

RESULT_Broadcast_Queue_Subscribe
Broadcast_Queue_Subscribe(Broadcast_Queue_t* broadcast, Broadcast_Queue_Member_t* member);


/** @brief Send data to every member.
 */
typedef enum {
  RESULT_Broadcast_Queue_Broadcast__Success,
  RESULT_Broadcast_Queue_Broadcast__Unknown_Result,
  RESULT_Broadcast_Queue_Broadcast__Failure_Send_All,
  RESULT_Broadcast_Queue_Broadcast__Failure_Send_Partial,
  RESULT_Broadcast_Queue_Broadcast__Broadcast_Null,
  RESULT_Broadcast_Queue_Broadcast__Subscribers_None
} RESULT_Broadcast_Queue_Broadcast_t;

RESULT_Broadcast_Queue_Broadcast_t
Broadcast_Queue_Broadcast(Broadcast_Queue_t* broadcast, void* data);





#endif /* BROADCAST_QUEUE_H */
