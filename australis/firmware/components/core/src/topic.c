/***********************************************************************************
 * @file        topic.c                                                            *
 * @brief       Implementation of the Topic-based Publish/Subscribe system.        *
 *                                                                                 *
 * @{                                                                              *
 ***********************************************************************************/

#include "stdint.h"
#include "stdbool.h"
#include "string.h"

#include "AustralisConfig.h"
#include "AustralisCore.h"
#include "_topic.h"
#include "topic.h"

#include "FreeRTOS.h" // IWYU pragma: keep
#include "portmacro.h"
#include "queue.h"

#ifndef coreMAX_SUBSCRIBERS
#error "coreMAX_SUBSCRIBERS must be defined in AustralisConfig.h"
#endif

#ifndef coreMAX_TOPICS
#error "coreMAX_TOPICS must be defined in AustralisConfig.h"
#endif

static Topic handle[coreMAX_TOPICS];
static uint8_t topicIndex = 0;

static Subscription subs[coreMAX_SUBSCRIBERS];
static uint8_t subIndex = 0;

static bool _Topic_isChild(TopicHandle_t child, TopicHandle_t parent);

/* =============================================================================== */
/**
 * @brief
 *
 **
 * =============================================================================== */
size_t Subscription_waitArticle(
  SubHandle_t handle,
  void *data,
  size_t length,
  TickType_t timeout
) {

  ASSERT(handle != NULL);
  ASSERT(data != NULL);
  ASSERT(length > 0);

  return xMessageBufferReceive(handle->inbox, data, length, timeout);
}

/* =============================================================================== */
/**
 * @brief
 *
 **
 * =============================================================================== */
TopicHandle_t Topic_new(
  const char *name,
  void *context,
  bool hidden,
  MessageHandler commentHandler
) {

  ASSERT(name != NULL);
  ASSERT(name[0] != '.');

  // Total topic count MUST be deterministic
  // and within bounds of array at run-time
  ASSERT(topicIndex <= coreMAX_TOPICS);

  TopicHandle_t newTopic  = &handle[topicIndex];
  newTopic->handleComment = commentHandler;
  newTopic->hidden        = hidden;
  newTopic->name          = name;

  // Track number of parents of
  // new topic to ensure single-ancestor
  uint8_t parents = 0;

  for (uint8_t i = 0; i < coreMAX_TOPICS && handle[i].initialised; i++) {
    TopicHandle_t topic = &handle[i];

    if (_Topic_isChild(newTopic, topic)) {
      // Topics may only have
      // a single parent
      ASSERT(parents++ <= 1);

      newTopic->parent = topic;
    }
  }

  // A topic without a parent (root topic) must not
  // contain any '.' delimiter in its string
  // i.e. "root_topic"  <- VALID
  //      ".root_topic" <- ERROR
  //      "root_topic." <- ERROR
  ASSERT(!newTopic->parent || (newTopic->parent && strchr(newTopic->name, '.')));

  newTopic->initialised = true;
  topicIndex++;

  return newTopic;
}

/* =============================================================================== */
/**
 * @brief
 *
 **
 * =============================================================================== */
bool Topic_publish(TopicHandle_t handle, uint8_t *article, size_t articleSize) {

  ASSERT(handle->initialised == true);

  // TODO: exception handling/error detection
  // i.e. what could go wrong in production operation?

  while (handle->head != NULL) {

    SubHandle_t sub = handle->head;

    // Send article to each subscriber inbox
    while (sub != NULL && sub->inbox != NULL) {
      // TODO: Maybe set error status on failure and return
      // it once complete?
      xMessageBufferSend(sub->inbox, article, articleSize, 2);
    }

    // Propogate comment upwards in
    // tree to all parent topics
    handle = handle->parent;
  }

  return true;
}

/* =============================================================================== */
/**
 * @brief  Subscribe to a topic using its unique string name.
 *
 **
 * =============================================================================== */
SubHandle_t Topic_subscribeByName(const char *name) {
  TopicAlias_t alias = Topic_getAlias(name);
  SubHandle_t sub    = Topic_subscribeByAlias(alias);

  return sub;
}

/* =============================================================================== */
/**
 * @brief  Create a new subscription to a topic using its alias.
 *
 **
 * =============================================================================== */
SubHandle_t Topic_subscribeByAlias(TopicAlias_t alias) {

  ASSERT(alias < topicIndex);

  // Total subscription count MUST be deterministic
  // and within bounds of array at run-time
  ASSERT(subIndex <= coreMAX_SUBSCRIBERS);

  Topic topic        = handle[alias];

  SubHandle_t newSub = &subs[subIndex++];

  // Assign head if not already
  if (topic.head == NULL) {
    topic.head = newSub;
  }

  // Update tail with new subscription
  if (topic.tail == NULL) {
    topic.tail = topic.head;
  } else {
    topic.tail->next = newSub;
    topic.tail       = newSub;
  }

  return newSub;
}

/* =============================================================================== */
/**
 * @brief  Create a new subscription to a topic using its handle.
 *
 **
 * =============================================================================== */
SubHandle_t Topic_subscribeByHandle(TopicHandle_t handle) {

  ASSERT(handle != NULL);

  SubHandle_t newSub = &subs[subIndex++];

  // Assign head if not already
  if (handle->head == NULL) {
    handle->head = newSub;
  }

  // Update tail with new subscription
  if (handle->tail == NULL) {
    handle->tail = handle->head;
  } else {
    handle->tail->next = newSub;
    handle->tail       = newSub;
  }

  return newSub;
}

/* =============================================================================== */
/**
 * @brief  Find a topic by its unique string name.
 *
 * @param  name The null-terminated string name of the topic to find.
 *
 * @return A `TopicHandle_t` to the found topic, or -1 if no topic
 *         with that name exists.
 **
 * =============================================================================== */
TopicAlias_t Topic_getAlias(const char *name) {

  ASSERT(name != NULL);

  for (uint8_t i = 0; i < coreMAX_TOPICS && handle[i].initialised; i++) {
    // Return topic handle if name matches query string
    if (!handle[i].hidden && strcmp(handle[i].name, name)) {
      return i;
    }
  }

  return -1;
}

// ALLOW FORMATTING
#ifndef __DOXYGEN__

/* =============================================================================== */
/**
 * @brief  Determines if one topic is a direct child of another.
 *
 * A topic 'A' is considered a direct child of topic 'B' if its name is
 * exactly "B.A". For example, "sensors.imu" is a direct child of "sensors",
 * but "sensors.imu.accel" is not. This function checks this specific
 * hierarchical relationship.
 *
 * @param  child  The handle of the potential child topic.
 * @param  parent The handle of the potential parent topic.
 *
 * @return `true` if `child` is a direct child of `parent`, `false` otherwise.
 **
 * =============================================================================== */
static bool _Topic_isChild(TopicHandle_t child, TopicHandle_t parent) {

  // Use parent topic string length
  // to determine child prefix
  size_t plen = strlen(parent->name);

  // Parent must be shorter
  if (strlen(child->name) >= plen) {
    return false;
  }

  // Check prefix matches
  if (strncmp(child->name, parent->name, plen) != 0) {
    return false;
  }

  // Ensure next char is '.'
  if (child->name[plen] != '.') {
    return false;
  }

  // Ensure there's no '.' after this segment
  const char *after = child->name + plen + 1;
  return strchr(after, '.') == NULL;
}

// TODO: Update below docstring

/* =============================================================================== */
/**
 * @brief
 *
 * @param
 *
 * @return
 **
 * =============================================================================== */
TopicHandle_t _Topic_getHandleFromAlias(TopicAlias_t alias) {
  ASSERT(alias < topicIndex);
  return &handle[alias];
}

#endif

/** @} */
