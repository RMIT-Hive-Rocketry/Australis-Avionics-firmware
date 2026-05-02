/***********************************************************************************
 * @file        AustralisCore.c                                                    *
 * @author      Matt Ricci                                                         *
 * @brief       Main application entry point and system initialization.            *
 ***********************************************************************************/

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "event_groups.h"
#include "message_buffer.h"
#include "semphr.h"
#include "stream_buffer.h"

#include "flashwrite.h"
#include "hdataacquisition.h"
#include "ldataacquisition.h"
#include "state.h"
#include "stateupdate.h"
#include "tasklist.h"

#include "rtos/_messagedispatch.h"

#include "AustralisConfig.h"
#include "AustralisCore.h"

// ------------------------------------------------------------

// RTOS event groups
EventGroupHandle_t xTaskEnableGroup; // 0: FLASH,  1: HIGHRES, 2: LOWRES, 3: LORA, 7: IDLE

/* =============================================================================== */
/**
 * @brief Run the Australis core

 * =============================================================================== */
void Australis_startCore(void) {

  // Start the scheduler
  vTaskStartScheduler();
}

/* =============================================================================== */
/**
 * @brief Initialisation of RTOS tasks
 *
 * Performs initial setup for various peripherals, ensuring all components
 * are ready for data acquisition and system state management. This task also
 * initializes RTOS event groups and message buffers to manage inter-task
 * communication.
 *
 * @todo Refactor context parameters that require write operations to intialise as
 *       pointers within their respective structs.
 *
 * @return void
 * =============================================================================== */
void Australis_init() {

#if coreUSE_TRACE == 1
  xTraceEnable(TRC_START);
#endif

  // TODO: Replace usage of event groups for xTaskEnableGroup with direct-to-task notifications
  xTaskEnableGroup = xEventGroupCreate(); // 0: FLASH,  1: HIGHRES, 2: LOWRES, 3: LORA, 7: IDLE

  // Initialise core state
  State_init();

  xTaskCreate(vMessageDispatcher, "MessageDispatch", 512, NULL, configMAX_PRIORITIES - 7, TaskList_new());

#if (coreTASK_ENABLE == 1)
  xTaskCreate(vHDataAcquisition, "HDataAcq", 512, NULL, configMAX_PRIORITIES - 2, TaskList_new());
  xTaskCreate(vLDataAcquisition, "LDataAcq", 512, NULL, configMAX_PRIORITIES - 3, TaskList_new());
  xTaskCreate(vStateUpdate, "StateUpdate", 512, NULL, configMAX_PRIORITIES - 4, TaskList_new());

#if coreFLASH_ENABLE == 1
  xTaskCreate(vFlashBuffer, "FlashData", 512, NULL, configMAX_PRIORITIES - 1, TaskList_new());
  xTaskCreate(vIdle, "Idle", 256, NULL, tskIDLE_PRIORITY, TaskList_new());
#endif

#endif
}
