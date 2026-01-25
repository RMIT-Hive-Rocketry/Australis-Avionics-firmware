/***********************************************************************************
 * @file        dummy.c                                                            *
 * @author      Matt Ricci                                                         *
 * @addtogroup  Shell                   																					 *
 *                                                                                 *
 * @{                                                                              *
 ***********************************************************************************/

#include "stdint.h"
#include "math.h"
#include "stdio.h"

#include "shell.h"
#include "devicelist.h"

#include "accelerometer.h"
#include "barometer.h"
#include "gyroscope.h"

#include "accelX.h"
#include "accelY.h"
#include "accelZ.h"

#include "gyroX.h"
#include "gyroY.h"
#include "gyroZ.h"

#include "press.h"

static void Dummy_exec(UART_t *uart, char *flags);

DEFINE_PROGRAM_HANDLE("dummy", Dummy_exec, NULL)

static int idxAccel = 0;
static int idxBaro  = 0;
static int idxGyro  = 0;

typedef union {
  uint32_t u32;
  float f32;
} fUnion;

static void *prev_readRawAccel;
static void readRawAccel(Accel_t *accel, uint8_t *bytes) {
  if (idxAccel + 1 < accelX_finish
      && idxAccel + 1 < accelY_finish
      && idxAccel + 1 < accelZ_finish) {
    int16_t rawX = roundf(
      ((fUnion){.u32 = ((uint32_t)accelX[idxAccel + 1] << 16) | accelX[idxAccel]}).f32
      / accel->sensitivity
    );

    int16_t rawY = roundf(
      ((fUnion){.u32 = ((uint32_t)accelY[idxAccel + 1] << 16) | accelY[idxAccel]}).f32
      / accel->sensitivity
    );

    int16_t rawZ = roundf(
      ((fUnion){.u32 = ((uint32_t)accelZ[idxAccel + 1] << 16) | accelZ[idxAccel]}).f32
      / accel->sensitivity
    );

    bytes[0]  = ((uint8_t *)&rawX)[1];
    bytes[1]  = ((uint8_t *)&rawX)[0];
    bytes[2]  = ((uint8_t *)&rawY)[1];
    bytes[3]  = ((uint8_t *)&rawY)[0];
    bytes[4]  = ((uint8_t *)&rawZ)[1];
    bytes[5]  = ((uint8_t *)&rawZ)[0];
    idxAccel += sizeof(uint16_t);
  }
}

static void *prev_readRawGyro;
static void readRawGyro(Gyro_t *gyro, uint8_t *bytes) {
  if (idxGyro + 1 < gyroX_finish
      && idxGyro + 1 < gyroY_finish
      && idxGyro + 1 < gyroZ_finish) {
    int16_t rawX = roundf(
      ((fUnion){.u32 = ((uint32_t)gyroX[idxGyro + 1] << 16) | gyroX[idxGyro]}).f32
      / gyro->sensitivity
    );

    int16_t rawY = roundf(
      ((fUnion){.u32 = ((uint32_t)gyroY[idxGyro + 1] << 16) | gyroY[idxGyro]}).f32
      / gyro->sensitivity
    );

    int16_t rawZ = roundf(
      ((fUnion){.u32 = ((uint32_t)gyroZ[idxGyro + 1] << 16) | gyroZ[idxGyro]}).f32
      / gyro->sensitivity
    );

    bytes[0]  = ((uint8_t *)&rawX)[1];
    bytes[1]  = ((uint8_t *)&rawX)[0];
    bytes[2]  = ((uint8_t *)&rawY)[1];
    bytes[3]  = ((uint8_t *)&rawY)[0];
    bytes[4]  = ((uint8_t *)&rawZ)[1];
    bytes[5]  = ((uint8_t *)&rawZ)[0];
    idxGyro  += sizeof(uint16_t);
  }
}

static void *prev_readRawPress;
static void readRawPress(Baro_t *baro, uint8_t *bytes) {
  if (idxBaro + 1 < press_finish) {
    int32_t raw = (int32_t)roundf(
      ((fUnion){.u32 = ((uint32_t)press[idxBaro + 1] << 16) | press[idxBaro]}).f32
      / baro->sensitivity
    );

    bytes[0]  = ((uint8_t *)&raw)[2];
    bytes[1]  = ((uint8_t *)&raw)[1];
    bytes[2]  = ((uint8_t *)&raw)[0];
    idxBaro  += sizeof(uint16_t);
  }
}

/* =============================================================================== */
/**
 * @brief
 *
 *
 **
 * =============================================================================== */
static void Dummy_exec(UART_t *uart, char *flags) {
  Baro_t *baro   = DeviceList_getDeviceHandle(DEVICE_BARO).device;
  Accel_t *accel = DeviceList_getDeviceHandle(DEVICE_ACCEL).device;
  Gyro_t *gyro   = DeviceList_getDeviceHandle(DEVICE_GYRO).device;

  char *str      = NULL;

  if (!baro || !accel || !gyro) {
    str = "Error: devices not initialised";
    goto CMD_EXIT;
  }

  if (prev_readRawAccel || prev_readRawGyro || prev_readRawPress) {
    str      = "Error: dummy data already injected. Resetting indices";
    idxAccel = 0;
    idxBaro  = 0;
    idxGyro  = 0;
    goto CMD_EXIT;
  }

  prev_readRawAccel   = accel->readRawBytes;
  accel->readRawBytes = readRawAccel;

  prev_readRawGyro    = gyro->readRawBytes;
  gyro->readRawBytes  = readRawGyro;

  prev_readRawPress   = baro->readRawPress;
  baro->readRawPress  = readRawPress;

CMD_EXIT:
  if (str) {
    uart->println(uart, str);
  }
}

/** @} */
