/***********************************************************************************
 * @file        launch.c                                                           *
 * @author      Matt Ricci                                                         *
 * @addtogroup  Shell                   																					 *
 *                                                                                 *
 * @{                                                                              *
 ***********************************************************************************/

#include "AustralisConfig.h"

#include "devicelist.h"
#include "shell.h"
#include "sensors.h"

#include "accelerometer.h"

static void Launch_exec(UART_t *uart, char *);

DEFINE_PROGRAM_HANDLE("launch", Launch_exec, NULL)

/* =============================================================================== */
/**
 * @brief
 **
 * =============================================================================== */
static void Launch_exec(UART_t *uart, char *flags) {
  Accel_t *accel           = DeviceList_getDeviceHandle(DEVICE_ACCEL).device;
  accel->accelData[ZINDEX] = ACCEL_LAUNCH;
  TaskHandle_t handle      = xTaskGetHandle("StateUpdate");
  xTaskAbortDelay(handle);
}

/** @} */
