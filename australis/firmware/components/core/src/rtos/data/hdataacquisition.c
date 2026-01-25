/***********************************************************************************
 * @file        hDataAcquisition.c                                                 *
 * @author      Matt Ricci                                                         *
 * @addtogroup  RTOS			                                                         *
 *                                                                                 *
 * @{                                                                              *
 ***********************************************************************************/

#include "FreeRTOS.h"
#include "event_groups.h"
#include "groups.h"
#include "message_buffer.h"
#include "semphr.h"

#include "dataframe.h"
#include "devicelist.h"
#include "membuff.h"
#include "sensors.h"

#include "state.h"
#include "accelerometer.h"
#include "gyroscope.h"
#include "hdataacquisition.h"

#include "quaternion.h"

extern EventGroupHandle_t xTaskEnableGroup;

#define ACCEL_MOTION_THRESHOLD 1.125f
#define GYRO_MOTION_THRESHOLD  2.0f

#define SQ(x)                  (x * x)
#define EWMA(a, ewma, x)       (((1 - a) * ewma) + (a * x))
#define MAG(v)                 (sqrtf(SQ(v[0]) + SQ(v[1]) + SQ(v[2])))

#define SWAP_AXES(device, oldIdx, newIdx)        \
  {                                              \
    uint8_t temp         = device->axes[oldIdx]; \
    device->axes[oldIdx] = device->axes[newIdx]; \
    device->axes[newIdx] = temp;                 \
  }

float gyroBiasX     = 0.0f;
float gyroBiasY     = 0.0f;
float gyroBiasZ     = 0.0f;
uint32_t numSamples = 0;

/* =============================================================================== */
/**
 * @brief High-frequency data acquisition task.
 *
 * Acquires sensor data at a 500Hz rate. It selects the appropriate accelerometer
 * based on current data, processes sensor data and appends the processed data
 * to a dataframe. Optionally, dummy data can be used for testing if the `DUMMY`
 * macro is defined.
 *
 * Quaternion integration and tilt angle calculations are performed if enabled.
 *
 * @todo Add definition for sample period and replace assignments for dt and
 *       frequency (e.g. dt = 1/SAMPLE_PERIOD_HIGH;).
 **
 * =============================================================================== */
void vHDataAcquisition(void *argument) {
  float dt                    = 0.002;

  const TickType_t xFrequency = pdMS_TO_TICKS(2); // 500Hz
  const TickType_t blockTime  = pdMS_TO_TICKS(0);

  // Devices
  Accel_t *hAccel = DeviceList_getDeviceHandle(DEVICE_ACCEL_HIGH).device;
  Accel_t *lAccel = DeviceList_getDeviceHandle(DEVICE_ACCEL_LOW).device;
  Gyro_t *gyro    = DeviceList_getDeviceHandle(DEVICE_GYRO).device;

  // Selected accelerometer (high/low)
  DeviceHandle_t *accelHandlePtr = DeviceList_getDeviceHandlePointer(DEVICE_ACCEL);
  Accel_t *accel                 = accelHandlePtr->device;

  State *state                   = State_getState();

  // Average weighted vector magnitudes
  float accelEWMA = 0; // Moving average for acceleration vector magnitude
  float gyroEWMA  = 0; // Moving average for gyroscope rate vector magnitude

  for (;;) {
    // Block until 2ms interval
    TickType_t xLastWakeTime = xTaskGetTickCount();
    vTaskDelayUntil(&xLastWakeTime, xFrequency);

    // --- Sample device measurements ---
    taskENTER_CRITICAL();
    lAccel->update(lAccel);
    hAccel->update(hAccel);
    gyro->update(gyro);
    taskEXIT_CRITICAL();

    // --- Select which accelerometer to use ---
    accelHandlePtr->device = (accel->accelData[ZINDEX] < 15) ? lAccel : hAccel;

    // --- Calibrate devices before launch ---
    if (state->flightState == PRELAUNCH) {

      // Calculate moving average of acceleration vector magnitude
      accelEWMA = EWMA(0.5, accelEWMA, MAG(accel->accelData));

      // Calculate moving average of gyroscope rate vector magnitude
      gyroEWMA = EWMA(0.5, gyroEWMA, MAG(gyro->gyroData));

      // --- Perform calibration whilst stationary ---
      //
      // Here, the rocket is determined to be "stationary" if the moving
      // average magnitude of gyroscope and accelerometer measurement
      // vectors are below the threshold.
      //
      // Gyroscope bias is estimated as a cumulative average of all
      // gyroscope readings whilst stationary.
      //
      // Axis adjustments are performed as a function of the index of
      // accelerometer measurement with greatest magnitude. All devices
      // have the same adjustment performed to maintain equivalence.

      if (accelEWMA < ACCEL_MOTION_THRESHOLD && gyroEWMA < GYRO_MOTION_THRESHOLD) {
        // Cumulative sum gyro bias samples
        gyro->bias[0] += 0.5 * (gyro->gyroData[0]) * dt;
        gyro->bias[1] += 0.5 * (gyro->gyroData[1]) * dt;
        gyro->bias[2] += 0.5 * (gyro->gyroData[2]) * dt;

        int oldIdx     = 0;
        // Determine current Z axis index
        for (; oldIdx < ZINDEX; oldIdx++) {
          if (accel->axes[oldIdx] == ZINDEX) {
            break;
          }
        }

        int newIdx = ZINDEX;
        // Determine current largest axis of acceleration
        for (int i = 0; i < ZINDEX; i++) {
          float indexedAxisAbsolute = fabs(accel->accelData[accel->axes[i]]);
          float currentAxisAbsolute = fabs(accel->accelData[accel->axes[newIdx]]);
          if (indexedAxisAbsolute > currentAxisAbsolute) {
            newIdx = i;
          }
        }

        // Swap indices of current Z axis and axis of largest magnitude
        SWAP_AXES(hAccel, oldIdx, newIdx)
        SWAP_AXES(lAccel, oldIdx, newIdx)
        SWAP_AXES(gyro, oldIdx, newIdx)

        // Invert Z-axis sign if necessary
        if (accel->accelData[ZINDEX] < 0) {
          lAccel->sign[ZINDEX] *= -1;
          hAccel->sign[ZINDEX] *= -1;
          gyro->sign[ZINDEX]   *= -1;
        }
      }
    }

    // --- Add sensor data to dataframe ---
    state->mem.append(&state->mem, HEADER_HIGHRES);
    state->mem.appendBytes(&state->mem, accel->rawAccelData, accel->dataSize);
    state->mem.appendBytes(&state->mem, gyro->rawGyroData, gyro->dataSize);

    // --- Calculate state variables ---
    EventBits_t uxBits = xEventGroupWaitBits(xTaskEnableGroup, GROUP_TASK_ENABLE_HIGHRES, pdFALSE, pdFALSE, blockTime);
    if (uxBits & GROUP_TASK_ENABLE_HIGHRES) {
      // Integrate attitude quaternion from rotations
      Quaternion qDot = Quaternion_new();
      qDot.fromEuler(
        &qDot,
        (float)(dt * gyro->gyroData[ROLL_INDEX]),
        (float)(dt * gyro->gyroData[PITCH_INDEX]),
        (float)(dt * gyro->gyroData[YAW_INDEX])
      );
      state->rotation = Quaternion_mul(&state->rotation, &qDot);
      state->rotation.normalise(&state->rotation); // New attitude quaternion

      // Apply rotation to z-axis unit vector
      state->rotation.fRotateVector3D(&state->rotation, state->launchAngle, state->attitude);

      // Calculate tilt angle
      // tilt = cos^-1(attitude · initial)
      state->cosine        = state->launchAngle[0] * state->attitude[0] + state->launchAngle[1] * state->attitude[1] + state->launchAngle[2] * state->attitude[2];
      state->tilt          = acosf(state->cosine) * (180 / 3.14159265);

      state->flightTimeMs += 2;
    }
  }
}

/** @} */
