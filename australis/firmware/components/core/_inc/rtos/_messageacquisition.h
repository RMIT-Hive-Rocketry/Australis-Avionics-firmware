// ALLOW FORMATTING
#ifndef _MESSAGEACQUISITION_H
#define _MESSAGEACQUISITION_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "topic.h"

void vMessageAcquirer(void *argument);

bool queueAcquisition(TopicHandle_t topic);

#endif
