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
#include "packet.h"

#include "can.h"
#include "canpub.h"

#include "flashwrite.h"

#include "sam_m10q.h"
#include "devicelist.h"

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
  //NVIC_SetPriority(USART1_IRQn, 10);
  //NVIC_EnableIRQ(USART1_IRQn);
  NVIC_SetPriority(USART3_IRQn, 11);
  NVIC_EnableIRQ(USART3_IRQn);
  EXTI->RTSR        |= 0x02;
  EXTI->IMR         |= 0x02;
  SYSCFG->EXTICR[0] &= ~0xF0;
  SYSCFG->EXTICR[0] |= 0x30;
  __enable_irq();

  // Task deletion is causing exit to WWDG.
  vTaskDelete(NULL);
  //vTaskSuspend(0);
  // for (;;) {
  //   ;
  // }
}

void EXTI1_IRQHandler(void *argument) {
  (void)argument;

  loraPub_interrupt();
  EXTI->PR |= (0x02);
}

void USART1_IRQHandler(void *argument) {
  (void)argument;

  pubShellRxInterrupt();
}

void USART3_IRQHandler(void *argument) {
  (void)argument;

  SAM_M10Q_UART_Interrupt();
}



void vAerobrakesSendData(void *argument) {

  const TickType_t xFrequency = pdMS_TO_TICKS(20); // 50Hz

  while (1)
    {
      State *state = State_getState();

      // Send CAN packets to the aerobrakes.
      // In the aerobrakes firmware, data is cast to an integer to be used for
      // a lookup table that controls the aerobrakes servo. For greater precision,
      // the values are multiplied by ten before being sent.
      switch (state->flightState) {
      case LAUNCH:
      case COAST:
      case APOGEE:
      case DESCENT:
        CAN_Packet packet;
        packet.id = CAN_ID_AB_Data;

        // Fill data
        uint16_t* can_altitude   = (uint16_t*)&packet.data.byte[0];
        uint16_t* can_velocity   = (uint16_t*)&packet.data.byte[2];
        _Float32* can_tilt_angle = (_Float32*)&packet.data.byte[4];

        *can_altitude   = (uint16_t)(10.0 * state->altitude);
        *can_velocity   = (uint16_t)(10.0 * state->velocity);
        *can_tilt_angle = state->tilt;

        packet.data.length  = 8;

        CAN_Transmission_Queue_Add(&packet);
      }

      vTaskDelay(xFrequency);
    }

}



void vGPS_Aquire(void *argument) {

  SAM_M10Q_t* gps = DeviceList_getDeviceHandle(DEVICE_GPS).device;
  gps->taskHandle = xTaskGetCurrentTaskHandle();
  
  while (1)
    {      
      // Await a notification.
      xTaskNotifyWait(0,0,NULL,portMAX_DELAY);
      gps->parse(gps);
    }
    
}



void vBroadcastCallsign(void *argument) {
  
  LoRa_Message_t message;
  message.length = 6;
  memcpy(message.data, "KR4MFM", 6);

  for (;;)
    {
      // Send packet comment to LoRa author
      xQueueSend(Queue_LoRa_Transmit, &message, 0);

      // Every minute, send our callsign.
      vTaskDelay(pdMS_TO_TICKS(1000*60));
    }
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

  // xTaskCreate(vAerobrakesSendData, "AerobrakesData", 256, NULL, configMAX_PRIORITIES - 1, TaskList_new());
  // xTaskCreate(vCanTransmit, "CAN Transmit", 256, NULL, configMAX_PRIORITIES - 1, TaskList_new());
  
  TaskHandle_t interruptTaskHandle;
  xTaskCreate(vEnableInterrupts, "interrupts", 128, NULL, tskIDLE_PRIORITY + 1, &interruptTaskHandle);

  xTaskCreate(vFlashBuffer, "Flash Buffer", 256, NULL, tskIDLE_PRIORITY + 1, TaskList_new());

  xTaskCreate(vBroadcastCallsign, "Callsign Broadcast", 256, NULL, tskIDLE_PRIORITY + 1, TaskList_new());

    
  xTaskCreate(vGPS_Aquire, "GPS Acquire", 512, NULL, tskIDLE_PRIORITY + 2, TaskList_new());


  return true;
}
