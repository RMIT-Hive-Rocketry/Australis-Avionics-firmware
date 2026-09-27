/***********************************************************************************
 * @file        tasks.c                                                            *
 * @author      Matt Ricci                                                         *
 * @brief                                                                          *
 ***********************************************************************************/

#include "FreeRTOS.h"
#include "task.h"

#include "stdbool.h"

#include "devices.h"

#include "canpub.h"
#include "gpiopin.h"
#include "tasklist.h"
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
  // NVIC_SetPriority(CAN1_RX0_IRQn, 11);
  // NVIC_EnableIRQ(CAN1_RX0_IRQn);
  __enable_irq();

  vTaskDelete(NULL);
}

void CAN1_RX0_IRQHandler(void) {
  pubCanInterrupt();
}

/* ============================================================================================== */
/**
 * @brief Initialise and store FreeRTOS task handles not handled by the Australis core.
 *
 * @return .
 **
 * ============================================================================================== */

bool initTasks(void) {
  xTaskCreate(vHeartbeatBlink, "HeartbeatBlink", 128, NULL, tskIDLE_PRIORITY + 1, TaskList_new());
  xTaskCreate(vStateLogic, "StateLogic", 128, NULL, tskIDLE_PRIORITY + 1, TaskList_new());
  // xTaskCreate(vCanReceive, "CanRx", 256, NULL, configMAX_PRIORITIES - 5, TaskList_new());
  //  xTaskCreate(vCanTransmit, "CanTx", 256, NULL, configMAX_PRIORITIES - 5, TaskList_new());

  TaskHandle_t interruptTaskHandle;
  xTaskCreate(vEnableInterrupts, "interrupts", 128, NULL, tskIDLE_PRIORITY, &interruptTaskHandle);
  return true;
}
