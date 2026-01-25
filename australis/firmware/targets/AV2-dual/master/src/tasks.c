/***********************************************************************************
 * @file        tasks.c                                                            *
 * @author      Matt Ricci                                                         *
 * @brief                                                                          *
 ***********************************************************************************/

#include "FreeRTOS.h"
#include "task.h"

#include "stdbool.h"

#include "shell.h"
#include "tasklist.h"
#include "devices.h"

#include "lorapub.h"
#include "canpub.h"
#include "groundcomms.h"
#include "statelogic.h"
#include "state.h"

void vHeartbeatBlink(void *argument) {
  (void)argument;

  State *state           = State_getState();
  GPIOpin_t heartbeatLED = GPIOpin_init(LED1_PORT, LED1_PIN, NULL);

  heartbeatLED.reset(&heartbeatLED);

  for (;;) {
    const TickType_t xFrequency = (state->flightState < LAUNCH)
                                  ? pdMS_TO_TICKS(675)
                                  : pdMS_TO_TICKS(168);

    TickType_t xLastWakeTime    = xTaskGetTickCount();
    vTaskDelayUntil(&xLastWakeTime, xFrequency);

    heartbeatLED.toggle(&heartbeatLED);
    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(130));
    heartbeatLED.toggle(&heartbeatLED);
    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(130));
    heartbeatLED.toggle(&heartbeatLED);
    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(125));
    heartbeatLED.toggle(&heartbeatLED);
    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(125));
  }
}

/**
 * @todo Refactor and document
 */
void vEnableInterrupts(void *argument) {
  (void)argument;

  __disable_irq();
  NVIC_SetPriority(EXTI1_IRQn, 9);
  NVIC_EnableIRQ(EXTI1_IRQn);
  NVIC_SetPriority(USART1_IRQn, 10);
  NVIC_EnableIRQ(USART1_IRQn);
  NVIC_SetPriority(USART3_IRQn, 11);
  NVIC_EnableIRQ(USART3_IRQn);
  EXTI->RTSR        |= 0x02;
  EXTI->IMR         |= 0x02;
  SYSCFG->EXTICR[0] &= ~0xF0;
  SYSCFG->EXTICR[0]  = 0x30;
  __enable_irq();

  vTaskDelete(NULL);
}

void EXTI1_IRQHandler(void *argument) {
  (void)argument;

  EXTI->PR |= (0x02);
  pubLoraInterrupt();
}

void USART1_IRQHandler(void *argument) {
  (void)argument;

  pubShellRxInterrupt();
}

/* ============================================================================================== */
/**
 * @brief Initialise and store FreeRTOS task handles not handled by the Australis core.
 *
 * @return .
 **
 * ============================================================================================== */

bool initTasks(void) {

  xTaskCreate(vHeartbeatBlink, "HeartbeatBlink", 128, NULL, configMAX_PRIORITIES - 1, TaskList_new());
  xTaskCreate(vStateLogic, "StateLogic", 128, NULL, tskIDLE_PRIORITY + 1, TaskList_new());

  xTaskCreate(vGroundCommStateMachine, "GroundComms", 512, NULL, configMAX_PRIORITIES - 5, TaskList_new());
  xTaskCreate(vLoRaTransmit, "LoraTx", 256, NULL, configMAX_PRIORITIES - 5, TaskList_new());
  xTaskCreate(vLoRaReceive, "LoraRx", 256, NULL, configMAX_PRIORITIES - 5, TaskList_new());
  xTaskCreate(vShellProcess, "ShellProcess", 256, NULL, configMAX_PRIORITIES - 6, TaskList_new());

  TaskHandle_t interruptTaskHandle;
  xTaskCreate(vEnableInterrupts, "interrupts", 128, NULL, tskIDLE_PRIORITY, &interruptTaskHandle);
  return true;
}
