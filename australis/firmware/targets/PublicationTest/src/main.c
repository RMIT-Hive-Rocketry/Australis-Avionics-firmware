/***********************************************************************************
 * @file        AV2m.c                                                             *
 * @author      Matt Ricci                                                         *
 ***********************************************************************************/

#include <stdint.h>

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "interrupts/interrupts.h"
#include "portmacro.h"
#include "projdefs.h"
#include "stm32f439xx.h"
#include "task.h"

#include "AustralisCore.h"
#include "AustralisConfig.h"
#include "messagedispatch.h"
#include "tasklist.h"
#include "topic.h"
#include "rcc.h"

#include "testpub.h"

TaskHandle_t publicationTestHandle;
TaskHandle_t mainTaskHandle;

void initRCC() {
  // Make sure all peripherals we will use are enabled
  RCC_START_PERIPHERAL(AHB1, GPIOA);
  RCC_START_PERIPHERAL(AHB1, GPIOB);
  RCC_START_PERIPHERAL(AHB1, GPIOC);
  RCC_START_PERIPHERAL(AHB1, GPIOD);
  RCC_START_PERIPHERAL(AHB1, GPIOE);
  RCC_START_PERIPHERAL(AHB1, GPIOF);
  RCC_START_PERIPHERAL(AHB1, GPIOG);
  RCC_START_PERIPHERAL(AHB1, GPIOH);
  RCC_START_PERIPHERAL(APB2, SPI1);
  RCC_START_PERIPHERAL(APB1, SPI3);
  RCC_START_PERIPHERAL(APB2, SPI4);
  RCC_START_PERIPHERAL(APB1, TIM6);
  RCC_START_PERIPHERAL(APB1, USART3);
  RCC_START_PERIPHERAL(APB2, USART1);
  RCC_START_PERIPHERAL(APB2, SYSCFG);
}

/* =============================================================================== */
/**
 * @brief
 *
 **
 * =============================================================================== */
void vMainTask(void *argument) {
  (void)argument;

  SubHandle_t root_bar_sub = Topic_subscribeByName("root.bar");
  TopicHandle_t root_bar   = Topic_getHandle("root.bar");

  struct foo {
    uint8_t x;
    uint8_t y;
  };

  uint8_t x = 255, y = 0;

  for (;;) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(100));

    struct foo f = {x--, y++};
    TickType_t t = xTaskGetTickCount();
    queueComment(root_bar, &f, sizeof(f));
  }
}

/* =============================================================================== */
/**
 * @brief
 *
 **
 * =============================================================================== */
// TODO: Make Topic_publish a public function, but make topics associated with 
// publications private to external sources; i.e. publication exposes a function
// to start a topic but does not expose the topic itself. 
// 
// This allows for private/protected handling of topics within publications to 
// prevent external tasks from publishing, but allows user-defined topics for
// generic pub/sub communication.
//
// NOTE: will require a modification to make the Topic struct definition public, 
// with an additional `protected` member flag to allow for hidden topics that 
// cannot be retrieved by Topic_get.

void initTopics() {
  PubTest_newTopic("root", EXTI15_10_IRQn, (PubTest_Context){NULL});
}

/* =============================================================================== */
/**
 * @brief
 *
 **
 * =============================================================================== */
void initTasks() {
  xTaskCreate(
    vMainTask,
    "main",
    128,
    NULL,
    configMAX_PRIORITIES - 1,
    TaskList_new()
  );
}

/* =============================================================================== */
/**
 * @brief
 *
 **
 * =============================================================================== */
int main() {

  // Initialise RCC
  initRCC();

  // Bring up core
  Australis_init();

  initTopics();
  initTasks();

  // Start the core
  //
  // This runs the FreeRTOS scheduler
  // and will never return.
  Australis_startCore();

  return 0;
}
