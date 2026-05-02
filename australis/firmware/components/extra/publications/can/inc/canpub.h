// ALLOW FORMATTING
#ifndef CANPUB_H
#define CANPUB_H

#include "broadcast_queue.h"
#include "can.h"


typedef enum : uint32_t {
  CAN_ID_AB_Data  = 0x601,
  CAN_ID_AB_Close = 0x602,
  CAN_ID_AB_Test  = 0x603
} CAN_ID_t;


#define CAN_LISTENERS_MAX 8
// How many instances of listening to CAN?

#define CAN_QUEUE_LENGTH 8

#define CAN_TRANSMISSION_QUEUE_LENGTH 16

typedef struct {
  CAN_ID_t id;
  QueueHandle_t queue;
} CAN_Queue_t;
//TODO: consider adding a set for concurrent read/write?


// CAN_Queue_Create
// Create a FreeRTOS queue to which received CAN packets matching the ID
// will be sent to.
typedef enum {
  Return_CAN_Queue_Create__Success,
  Return_CAN_Queue_Create__Error_QueueCreateStatic_Failed,
  Return_CAN_Queue_Create__Error_Too_Many_Listeners
} Return_CAN_Queue_Create_t;

Return_CAN_Queue_Create_t
CAN_Queue_Create(CAN_Queue_t* target, CAN_ID_t id);


extern QueueHandle_t CAN_Transmission_Queue;

// CAN_Transmission_Queue_Add
typedef enum {
  Return_CAN_Transmission_Queue_Add__Success,
  Return_CAN_Transmission_Queue_Add__Error_Queue_Full
} Return_CAN_Transmission_Queue_Add_t;

Return_CAN_Transmission_Queue_Add_t
CAN_Transmission_Queue_Add(CAN_Packet* packet);


void vCanTransmit(void *pvParameters);
void vCanReceive(void *pvParameters);
void CAN_setPeripheral(CAN_t peripheral);
void pubCanInterrupt();

#endif
