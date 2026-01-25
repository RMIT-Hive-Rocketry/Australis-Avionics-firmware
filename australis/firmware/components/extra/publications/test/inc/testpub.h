// ALLOW FORMATTING
#ifndef TESTPUB_H
#define TESTPUB_H

#include "stdbool.h"

#include "interrupts/interrupts.h"
#include "topic.h"

#define TESTPUB_MESSAGE_MAX 32

typedef struct {
  void *context;
} PubTest_Context;

bool PubTest_newTopic(char *topicName, InterruptIdentifier rxIntIdentifier, PubTest_Context context);

#endif
