/**
 * @file _topic.h
 */

// ALLOW FORMATTING
#ifndef PRIVATETOPIC_H
#define PRIVATETOPIC_H

#include "stddef.h"
#include "stdint.h"

#include "topic.h"
#include "stdbool.h"

#define WAIT_COMMENT xMessageBufferReceive

/**
 * @brief Internal representation of a Subscription instance.
 *
 */
typedef struct Subscription {
  SubHandle_t next;
  SubInbox_t inbox;
} Subscription;

TopicHandle_t _Topic_getHandleFromAlias(TopicAlias_t alias);

bool Topic_publish(TopicHandle_t handle, uint8_t *article, size_t articleSize);

#endif
