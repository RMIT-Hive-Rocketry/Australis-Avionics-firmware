/**
 * @file topic.h
 */

// ALLOW FORMATTING
#ifndef TOPIC_H
#define TOPIC_H

#include <stdbool.h>
#include <stddef.h>

#include "FreeRTOS.h" // IWYU pragma: keep
#include "portmacro.h"
#include "message_buffer.h"

#define WAIT_ARTICLE xMessageBufferReceive

/**
 *
 */
typedef MessageBufferHandle_t SubInbox_t;

/**
 *
 */
typedef struct Subscription *SubHandle_t;

/**
 *
 */
typedef struct Topic *TopicHandle_t;

/**
 *
 */
typedef int TopicAlias_t;

/**
 *
 */
typedef bool (*MessageHandler)(TopicHandle_t handle, void *data, size_t size);

/**
 *
 */
typedef bool (*AcquisitionHandler)(TopicHandle_t handle);

/**
 * @brief Internal representation of a Topic instance.
 *
 */
typedef struct Topic {
  TopicAlias_t alias;             //!<
  TopicHandle_t parent;           //!<
  const char *name;               //!<
  void *context;                  //!<
  bool initialised;               //!<
  bool hidden;                    //!<
  SubHandle_t head;               //!<
  SubHandle_t tail;               //!<
  size_t numSubscriptions;        //!< Number of subscriptions to the topic.
  MessageHandler handleComment;   //!<
  AcquisitionHandler acquireData; //!<
} Topic;

TopicHandle_t Topic_new(const char *name, void *context, bool hidden, MessageHandler commentHandler);

TopicAlias_t Topic_getAlias(const char *name);
SubHandle_t Topic_subscribeByAlias(TopicAlias_t alias);
SubHandle_t Topic_subscribeByHandle(TopicHandle_t handle);
SubHandle_t Topic_subscribeByName(const char *name);

size_t Subscription_waitArticle(SubHandle_t handle, void *data, size_t length, TickType_t timeout);

#endif
