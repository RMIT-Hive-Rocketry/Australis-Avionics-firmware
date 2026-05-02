/***********************************************************************************
 * @file        stateUpdate.c                                                      *
 * @author      Matt Ricci                                                         *
 * @addtogroup  RTOS			                                                         *
 *                                                                                 *
 * @{                                                                              *
 ***********************************************************************************/

#include <stdint.h>

#include "FreeRTOS.h" // IWYU pragma: keep
#include "portmacro.h"
#include "projdefs.h"
#include "semphr.h"
#include "message_buffer.h"
#include "event_groups.h"

#include "devicelist.h"

#include "dataframe.h"
#include "sensors.h"
#include "groups.h"

#include "accelerometer.h"
#include "state.h"
#include "membuff.h"
#include "stateupdate.h"

#include "AustralisConfig.h"

extern EventGroupHandle_t xTaskEnableGroup;
EventGroupHandle_t xFlightStateGroup;

/* =============================================================================== */
/**
 * @brief State update task.
 *
 * Handles transitions between different flight states, sends CAN messages
 * for aerobrakes and altitude data, and enables or disables various data
 * acquisition tasks based on the current state.
 *
 * @todo Add definition for update period and replace assignments for frequency
 *       (e.g. xFrequency = pdMS_TO_TICKS(STATE_UPDATE_PERIOD);).
 **
 * =============================================================================== */
void vStateUpdate(void *argument) {
  const TickType_t xFrequency = pdMS_TO_TICKS(20); // 50Hz

  xFlightStateGroup           = xEventGroupCreate();
  xEventGroupSetBits(xFlightStateGroup, FLIGHT_STATE_BIT_PRELAUNCH);

  float avgPressCurrent      = 0;
  float avgPressPrevious     = 0;
  float avgVelCurrent        = 0;
  float avgVelPrevious       = 0;

  State *state               = State_getState();

  DeviceHandle_t accelHandle = DeviceList_getDeviceHandle(DEVICE_ACCEL);
  Accel_t *accel             = accelHandle.device;

  for (;;) {
    // Block until 20ms interval
    TickType_t xLastWakeTime = xTaskGetTickCount();
    vTaskDelayUntil(&xLastWakeTime, xFrequency);

    switch (state->flightState) {
    case PRELAUNCH:
      if (accel->accelData[ZINDEX] >= ACCEL_LAUNCH) {
        xEventGroupSetBits(xTaskEnableGroup, GROUP_TASK_ENABLE_FLASH);   // Enable flash
        xEventGroupSetBits(xTaskEnableGroup, GROUP_TASK_ENABLE_HIGHRES); // Enable high resolution data acquisition
        xEventGroupSetBits(xTaskEnableGroup, GROUP_TASK_ENABLE_LOWRES);  // Enable low resolution data acquisition
        xEventGroupSetBits(xFlightStateGroup, FLIGHT_STATE_BIT_LAUNCH);
        state->flightState = LAUNCH;
      }
      break;

    case LAUNCH:
      state->avgVel.calculateMovingAverage(&state->avgVel, &avgVelCurrent);
      // Transition to motor burnout state on velocity decrease
      if ((avgVelCurrent - avgVelPrevious) < 0) {
        xEventGroupSetBits(xFlightStateGroup, FLIGHT_STATE_BIT_COAST);
        state->flightState = COAST;
      }
      avgVelPrevious = avgVelCurrent;
      break;

    case COAST:
      state->avgPress.calculateMovingAverage(&state->avgPress, &avgPressCurrent);
      // Transition to apogee state on three way vote of altitude, velocity, and tilt
      // apogee is determined as two of three conditions evaluating true
      if ((((avgPressCurrent - avgPressPrevious) > 0) + (state->tilt >= 90) + (state->velocity < 0.0f)) >= 2) {
        xEventGroupSetBits(xFlightStateGroup, FLIGHT_STATE_BIT_APOGEE);
        state->flightState = APOGEE;

        // Log apogee event to flash
        state->mem.appendBytes(&state->mem, (uint8_t[]){HEADER_EVENT_APOGEE, 0xB0, 0x0B}, 3);
        state->mem.appendBytes(
          &state->mem,
          (uint8_t *)&state->flightTimeMs,
          sizeof(state->flightTimeMs)
        );

        // Send transmission to trigger apogee E-matches
      }
      avgPressPrevious = avgPressCurrent;
      break;

    case APOGEE:
      // Deploy drogue chute
      //
      // Transition to descent state when below main deployment altitude
      if (state->altitude <= MAIN_ALTITUDE_METERS) {
        xEventGroupSetBits(xFlightStateGroup, FLIGHT_STATE_BIT_DESCENT);
        state->flightState = DESCENT;
        // Add descent event dataframe to buffer
      }
      break;

    case DESCENT:
      // Handle descent state actions
      break;
    }
  }
}

/** @} */
