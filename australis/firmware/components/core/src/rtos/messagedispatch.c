/***********************************************************************************
 * @file        .c                                                                 *
 * @author      Matt Ricci                                                         *
 ***********************************************************************************/

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "FreeRTOS.h" // IWYU pragma: keep
#include "portmacro.h"
#include "semphr.h"
#include "message_buffer.h"

#include "AustralisConfig.h"
#include "AustralisCore.h"
#include "_topic.h"
#include "topic.h"

#include "rtos/_messagedispatch.h"
#include "rtos/messagedispatch.h"

#ifndef coreMAX_MESSAGE_COUNT
#error "coreMAX_MESSAGE_COUNT must be defined in AustralisConfig.h"
#endif

#ifndef coreMAX_MESSAGE_SIZE
#error "coreMAX_MESSAGE_SIZE must be defined in AustralisConfig.h"
#endif

static MessageBufferHandle_t messageBuff;
static SemaphoreHandle_t buffMutex;

static bool queueDispatch(TopicHandle_t topic, MessageType type, void *data, size_t size);

// Convenience typedef to alias either handleArticle or handleComment
typedef bool (*DispatchHandler)(Message *, size_t);

static bool handleArticle(Message *msg, size_t size);
static bool handleComment(Message *msg, size_t size);

/* =============================================================================== */
/**
 * @brief
 *
 **
 * =============================================================================== */
bool queueComment(TopicAlias_t alias, void *data, size_t size) {
  return queueDispatch(_Topic_getHandleFromAlias(alias), MESSAGE_COMMENT, data, size);
}

#ifndef __DOXYGEN__

/* =============================================================================== */
/**
 * @brief
 *
 **
 * =============================================================================== */
bool queueArticle(TopicHandle_t handle, void *data, size_t size) {
  return queueDispatch(handle, MESSAGE_ARTICLE, data, size);
}

#endif

/* =============================================================================== */
/**
 * @brief
 *
 * TODO:
 * Add mechanism for timeout. User configures maximum message time, and any message
 * that exceeds it will be dropped.
 **
 * =============================================================================== */
void vMessageDispatcher(void *argument) {

  (void)argument;

  // Initialise the MessageBuffer structure with user configured size
  const size_t buffSize = (coreMAX_MESSAGE_COUNT * coreMAX_MESSAGE_SIZE);

  ASSERT(messageBuff = xMessageBufferCreate(buffSize));
  ASSERT(buffMutex = xSemaphoreCreateMutex());

  // dataBuff receives the raw bytes from
  // the buffer, type-punned to a Message
  // struct via msg
  uint8_t dataBuff[coreMAX_MESSAGE_SIZE];
  Message *msg = (Message *)dataBuff;

  for (;;) {

    size_t bytesReceived = xMessageBufferReceive(messageBuff, dataBuff, coreMAX_MESSAGE_SIZE, portMAX_DELAY);

    // Ignore loop if timed out
    if (!bytesReceived) {
      continue;
    }

    // Length of received data must be greater
    // than the Message header size...
    // (i.e. some data is available)
    ASSERT(bytesReceived > sizeof(Message));

    // Dispatched messages must be defined with a valid type...
    ASSERT(msg->type == MESSAGE_COMMENT || msg->type == MESSAGE_ARTICLE);

    // Dispatch message with appropriate method based on value of msg->type
    //
    // TODO:
    // Do something with returned bool, maybe ASSERT?
    // Depends on implementation of error checking in dispatch methods
    DispatchHandler handleDispatch = (msg->type == MESSAGE_COMMENT) ? handleComment : handleArticle;

    handleDispatch(msg, bytesReceived);
  }
}

// ALLOW FORMATTING
#ifndef __DOXYGEN__

/* =============================================================================== */
/**
 * @brief
 *
 **
 * =============================================================================== */
bool queueDispatch(TopicHandle_t topic, MessageType type, void *data, size_t size) {

  ASSERT(messageBuff != NULL); // Dispatcher must be initialised first...
  ASSERT(topic != NULL);       // Cannot dispatch to a NULL topic...

  uint8_t bytes[sizeof(Message) + size];
  Message *msg = (Message *)bytes;

  memcpy(&msg->data, data, size);
  msg->topic = topic;
  msg->type  = type;

  size_t bytesSent;

  // Attempt to send message to dispatch queue, blocking only on mutex
  xSemaphoreTake(buffMutex, portMAX_DELAY);
  bytesSent = xMessageBufferSend(messageBuff, msg, sizeof(Message) + size, 0);
  xSemaphoreGive(buffMutex);

  // TODO: Add dispatch error enum with defined return codes
  return bytesSent == (sizeof(Message) + size);
}

/* =============================================================================== */
/**
 * @brief
 *
 **
 * =============================================================================== */
static bool handleArticle(Message *msg, size_t size) {

  ASSERT(msg->topic != NULL);
  ASSERT(msg->data != NULL);

  TopicHandle_t handle = msg->topic;

  return Topic_publish(handle, msg->data, size - sizeof(Message));
}

/* =============================================================================== */
/**
 * @brief
 *
 **
 * =============================================================================== */
static bool handleComment(Message *msg, size_t size) {

  ASSERT(msg->topic != NULL);
  ASSERT(msg->data != NULL);

  TopicHandle_t topic = msg->topic;

  ASSERT(topic->initialised == true);
  ASSERT(topic->handleComment != NULL);

  // TODO: exception handling/error detection
  // i.e. what could go wrong in production operation?

  return topic->handleComment(topic->context, msg->data, size - sizeof(Message));
}

#endif
