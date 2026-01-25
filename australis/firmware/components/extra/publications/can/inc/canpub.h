// ALLOW FORMATTING
#ifndef CANPUB_H
#define CANPUB_H

#include "topic.h"
#include "can.h"

#define CAN_MSG_LENGTH 8

extern Topic *canTopic;

void vCanTransmit(void *pvParameters);
void vCanReceive(void *pvParameters);
void CAN_setPeripheral(CAN_t *peripheral);
void pubCanInterrupt();

#endif
