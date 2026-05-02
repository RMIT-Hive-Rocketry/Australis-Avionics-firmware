/**************************************************************************************************
 * @file  testpub.c                                                                               *
 *                                                                                                *
 * @{                                                                                             *
 **************************************************************************************************/

#include <stddef.h>
#include <stdint.h>

#include "testpub.h"

#include "stm32f439xx.h"

#include "AustralisConfig.h"
#include "AustralisCore.h"
#include "_topic.h"
#include "topic.h"

#include "rtos/_messagedispatch.h"

#include "interrupts/interrupts.h"

static bool PubTest_handleComment(TopicHandle_t topic, void *data, size_t size);
static void PubTest_handleArticle(void *context);

/* =============================================================================== */
/**
 * @brief
 *
 **
 * =============================================================================== */
bool PubTest_newTopic(
  char *topicName,
  InterruptIdentifier interruptId,
  PubTest_Context pubCtx
) {

  TopicHandle_t topicHandle = Topic_new(topicName, &pubCtx, true);

  InterruptContext context  = {.callback = PubTest_handleArticle, .context = &topicHandle};

  InterruptHandle handle    = {
       .context    = context,
       .type       = INTERRUPT_EXTI,
       .identifier = interruptId,
       .priority   = 9
  };

  Interrupt_registerHandle(handle);

  return true;
}

/* =============================================================================== */
/**
 * @brief
 *
 **
 * =============================================================================== */
static void PubTest_handleArticle(void *context) {
  PubTest_Context *pubCtx = context;
}

#ifndef __DOXYGEN__

/* =============================================================================== */
/**
 * @brief
 *
 **
 * =============================================================================== */
static bool PubTest_handleComment(TopicHandle_t topic, void *data, size_t size) {
  uint8_t (*commentData)[size] = (uint8_t (*)[size])data;
  uint32_t x                   = DWT->CYCCNT;
  uint32_t y                   = SysTick->VAL;
  return false;
}

#endif

/** @} */
