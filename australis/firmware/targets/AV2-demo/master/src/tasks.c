/***********************************************************************************
 * @file        tasks.c                                                            *
 * @author      Matt Ricci                                                         *
 * @brief                                                                          *
 ***********************************************************************************/

#include "FreeRTOS.h"
#include "stm32f439xx.h"
#include "task.h"

#include "stdbool.h"

#include "event_groups.h"

#include "shell.h"
#include "tasklist.h"
#include "devices.h"
#include "state.h"
#include "groups.h"
#include "devicelist.h"
#include "sensors.h"
#include "accelerometer.h"

#include "lorapub.h"
#include "groundcomms.h"
#include "statelogic.h"

extern EventGroupHandle_t xTaskEnableGroup;

void vHeartbeatBlink(void *argument) {

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
void vEnableInterrupts() {
  __disable_irq();
  NVIC_SetPriority(EXTI1_IRQn, 9);
  NVIC_EnableIRQ(EXTI1_IRQn);
  EXTI->RTSR        |= EXTI_RTSR_TR1;
  EXTI->IMR         |= EXTI_IMR_IM1;
  SYSCFG->EXTICR[0] &= ~SYSCFG_EXTICR1_EXTI1_Msk;
  SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI1_PD;

  NVIC_SetPriority(USART1_IRQn, 10);
  NVIC_EnableIRQ(USART1_IRQn);
  __enable_irq();

  vTaskDelete(NULL);
}

void vHandleGpioLink(void *argument) {
  (void)argument;

  GPIOpin_t link1 = GPIOpin_init(GPIOE, GPIO_PIN1, &GPIO_CONFIG_INPUT);
  GPIOpin_t link2 = GPIOpin_init(GPIOE, GPIO_PIN0, &GPIO_CONFIG_INPUT);

  for (;;) {
    const TickType_t xFrequency = pdMS_TO_TICKS(10);
    TickType_t xLastWakeTime    = xTaskGetTickCount();
    vTaskDelayUntil(&xLastWakeTime, xFrequency);

    if (link1.isSet(&link1)) {
      NVIC_SystemReset();
    } else if (link2.isSet(&link2)) {
      Accel_t *accel           = DeviceList_getDeviceHandle(DEVICE_ACCEL).device;
      accel->accelData[ZINDEX] = ACCEL_LAUNCH;
      TaskHandle_t handle      = xTaskGetHandle("StateUpdate");
      xTaskAbortDelay(handle);
    }
  }
}

void EXTI1_IRQHandler() {
  EXTI->PR |= (0x02);
  loraPub_interrupt();
}

void USART1_IRQHandler() {
  pubShellRxInterrupt();
}

/* ============================================================================================== */
/**
 * @brief Initialise and store FreeRTOS task handles not handled by the Australis core.
 *
 * @return .
 **
 * ============================================================================================== */

bool initTasks() {

  xTaskCreate(vHeartbeatBlink, "HeartbeatBlink", 128, NULL, configMAX_PRIORITIES - 1, TaskList_new());
  xTaskCreate(vStateLogic, "StateLogic", 128, NULL, tskIDLE_PRIORITY + 1, TaskList_new());

  xTaskCreate(vGroundCommStateMachine, "GroundComms", 512, NULL, configMAX_PRIORITIES - 5, TaskList_new());
  xTaskCreate(vLoRaTransmit, "LoraTx", 256, NULL, configMAX_PRIORITIES - 5, TaskList_new());
  xTaskCreate(vLoRaReceive, "LoraRx", 256, NULL, configMAX_PRIORITIES - 5, TaskList_new());
  xTaskCreate(vShellProcess, "ShellProcess", 256, NULL, configMAX_PRIORITIES - 6, TaskList_new());

  TaskHandle_t interruptTaskHandle;
  xTaskCreate(vEnableInterrupts, "interrupts", 128, NULL, tskIDLE_PRIORITY, &interruptTaskHandle);

  xTaskCreate(vHandleGpioLink, "gpiolink", 128, NULL, configMAX_PRIORITIES - 5, TaskList_new());

  return true;
}
