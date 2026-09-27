/***********************************************************************************
 * @file        stateupdate.c                                                      *
 * @author      Matt Ricci                                                         *
 * @brief                                                                          *
 ***********************************************************************************/

#include "FreeRTOS.h"

#include "gpiopin.h"
#include "stateupdate.h"

#include "accelerometer.h"
#include "sensors.h"

#include "devicelist.h"
#include "devices.h"

/* ============================================================================================== */
/**
 * @brief
 *
 * @return .
 **
 * ============================================================================================== */
void vStateLogic(void *argument) {
  (void)argument;

  GPIOpin_t stateLED = GPIOpin_init(LED2_PORT, LED2_PIN, NULL);
  stateLED.reset(&stateLED);

  Accel_t *accel = DeviceList_getDeviceHandle(DEVICE_ACCEL).device;

  for (;;) {

    while (!(xEventGroupWaitBits(xFlightStateGroup, FLIGHT_STATE_BIT_LAUNCH, pdFALSE, pdFALSE, 2) & FLIGHT_STATE_BIT_LAUNCH)) {
      // clang-format off
      if (accel->accelData[ZINDEX] < accel->accelData[XINDEX]
       || accel->accelData[ZINDEX] < accel->accelData[YINDEX])
        stateLED.set(&stateLED);
      else
        stateLED.reset(&stateLED);
      // clang-format on
    }
    stateLED.reset(&stateLED);

    xEventGroupWaitBits(xFlightStateGroup, FLIGHT_STATE_BIT_APOGEE, pdFALSE, pdFALSE, portMAX_DELAY);
    stateLED.set(&stateLED);

    TickType_t xLastWakeTime = xTaskGetTickCount();
    vTaskDelayUntil(&xLastWakeTime, portMAX_DELAY);
  }
}
