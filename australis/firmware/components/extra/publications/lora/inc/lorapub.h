// ALLOW FORMATTING
#ifndef LORAPUB_H
#define LORAPUB_H

#include "topic.h"
#include "lora.h"
#include "gpiopin.h"

typedef struct {
  LoRa_t *transceiver;
  GPIOpin_t *rfToggle;
  volatile bool *ready;
} PubLora_Context;

bool PubLora_startup(TopicHandle_t topic, void *context);

#endif
