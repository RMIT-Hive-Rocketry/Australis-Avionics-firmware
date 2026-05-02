/***********************************************************************************
 * @file        .c                                                                 *
 * @author      Matt Ricci                                                         *
 ***********************************************************************************/

#include <stddef.h>

#include "FreeRTOS.h" // IWYU pragma: keep
#include "queue.h"

#include "AustralisConfig.h"
#include "AustralisCore.h"
#include "_topic.h"
#include "topic.h"

// TODO:
// Maybe for private header naming convention, leave the
// header with the same name but change the directory and
// enforce relative imports only?
// i.e. #include "_rtos/messageacquisition.h"
#include "rtos/_messageacquisition.h"

#ifndef coreMAX_MESSAGE_COUNT
#error "coreMAX_MESSAGE_COUNT must be defined in AustralisConfig.h"
#endif

static QueueHandle_t requestQueue;

/* =============================================================================== */
/**
 * @brief
 *
 **
 * =============================================================================== */
bool queueAcquisition(TopicHandle_t topic) {
  return false;
}

/* =============================================================================== */
/**
 * @brief
 *
 **
 * =============================================================================== */
void vMessageAcquirer(void *argument) {

  (void)argument;

  ASSERT(requestQueue = xQueueCreate(coreMAX_MESSAGE_COUNT, sizeof(AcquisitionHandler)));

  for (;;) {
  }
}
