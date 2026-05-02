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

#include "uart.h"
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

  GPIOpin_t stateLED = GPIOpin_init(LED2_PORT, LED2_PIN, NULL);
  stateLED.reset(&stateLED);

  UART_t *usb         = DeviceList_getDeviceHandle(DEVICE_UART_USB).device;
  Accel_t *accel      = DeviceList_getDeviceHandle(DEVICE_ACCEL).device;

  GPIOpin_t camPower1 = GPIOpin_init(GPIOH, GPIO_PIN4, NULL);
  camPower1.set(&camPower1);

  // Camera power channel 2
  GPIOpin_t camPower2 = GPIOpin_init(GPIOH, GPIO_PIN5, NULL);
  camPower2.set(&camPower2);

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
    // usb->println(usb, "switch");
    stateLED.set(&stateLED);

    TickType_t xLastWakeTime = xTaskGetTickCount();
    vTaskDelayUntil(&xLastWakeTime, portMAX_DELAY);
  }
}
