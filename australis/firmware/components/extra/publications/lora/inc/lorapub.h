// ALLOW FORMATTING
#ifndef LORAPUB_H
#define LORAPUB_H

#include "topic.h"
#include "lora.h"
#include "gpiopin.h"
#include "queue.h"

#include "broadcast_queue.h"
#include "AustralisConfig.h"

typedef struct {
  size_t length;
  uint8_t data[LORA_MSG_LENGTH];
} LoRa_Message_t;

// Public queue for submitting data for transmission.
extern QueueHandle_t Queue_LoRa_Transmit;
#define QUEUE_LORA_LENGTH 32

// Public broadcast queue for those interested in LoRa messages.
extern Broadcast_Queue_t BQueue_LoRa_Received;

bool PubLora_startup(TopicHandle_t topic, void *context);


void vLoRaTransmit(void *argument);
void vLoRaReceive(void *argument);
void loraPub_interrupt(void);
void loraPub_setRfToggle(GPIOpin_t *rfToggle_);


void loraPub_setTransceiver(LoRa_t *transceiver_);
LoRa_t *loraPub_getTransceiver();

#endif
