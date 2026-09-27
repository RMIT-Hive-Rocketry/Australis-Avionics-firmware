/************************************************************************
 * @file        lDataAcquisition.c                                                 *
 * @author      Matt Ricci                                                         *
 * @addtogroup  RTOS			                                                         *
 *                                                                                 *
 * @{                                                                              *
 ***********************************************************************************/

#include "math.h"

#include "FreeRTOS.h"
#include "event_groups.h"
#include "message_buffer.h"
#include "semphr.h"

#include "dataframe.h"
#include "groups.h"

#include "devicelist.h"
#include "kalmanfilter.h"
#include "membuff.h"
#include "sensors.h"
#include "barometer.h"
#include "accelerometer.h"
#include "state.h"
#include "uart.h"

#include "ldataacquisition.h"

extern EventGroupHandle_t xTaskEnableGroup;

/* =============================================================================== */
/**
 * @brief Low-frequency data acquisition and altitude estimation function.
 *
 * Performs data acquisition for barometric pressure at a 50Hz rate.
 * Altitude is calculated from barometric pressure using the hypsometric formula
 * Kalman filter state matrices (A, Q, R, P) are initialized within the function.
 * Optionally, dummy data can be used for testing if the `DUMMY` macro is defined.
 *
 * Velocity and altitude state estimates are calculated with the Kalman filter
 * if enabled.
 *
 * @todo Add definition for sample period and replace assignments for dt and
 *       frequency (e.g. dt = 1/SAMPLE_PERIOD_LOW;).
 **
 * =============================================================================== */
void vLDataAcquisition_Primary(void *argument) {
  float dt = 0.020;
  KalmanFilter kf;
  KalmanFilter_init(&kf);

  //! @todo Move kalman filter matrices into context struct
  // Initialise filter parameters
  float A[9] = {
    1.0, dt, 0.5 * (dt * dt),
    0.0, 1.0, dt,
    0.0, 0.0, 1.0
  };
  kf.A.pData = A;
  float Q[9] = {
    99.52, 0.0, 0.0,
    0.0, 1.42, 0.0,
    0.0, 0.0, 6.27
  };
  kf.Q.pData = Q;
  float R[4] = {
    97.92, 0.0,
    0.0, 0.61
  };
  kf.R.pData = R;
  float P[9] = {
    1, 0.0, 0.0,
    0.0, 0.1, 0.0,
    0.0, 0.0, 100.0
  };
  kf.P.pData = P;

  // Initialise measurement matrix
  arm_matrix_instance_f32 z;
  float zData[2] = {0.0, 0.0};
  arm_mat_init_f32(&z, 2, 1, zData);

  TickType_t xLastWakeTime;
  const TickType_t xFrequency = pdMS_TO_TICKS(20); // 50Hz
  const TickType_t blockTime  = pdMS_TO_TICKS(0);

  UART_t *usb                 = DeviceList_getDeviceHandle(DEVICE_UART_USB).device;
  Baro_t *baro                = DeviceList_getDeviceHandle(DEVICE_BARO).device;
  Accel_t *accel              = DeviceList_getDeviceHandle(DEVICE_ACCEL).device;

  State *state                = State_getState();

  // Create CAN queues to receive data.
  CAN_Queue_t CAN_Queue_Baro_Ground_Pressure;
  CAN_Queue_Create(&CAN_Queue_Baro_Ground_Pressure, CAN_ID_BARO_GROUND_PRESSURE);

  CAN_Queue_t CAN_Queue_Baro_Pressure;
  CAN_Queue_Create(&CAN_Queue_Baro_Pressure,        CAN_ID_BARO_PRESSURE);
  
  CAN_Queue_t CAN_Queue_Baro_Temperature;
  CAN_Queue_Create(&CAN_Queue_Baro_Temperature,     CAN_ID_BARO_TEMPERATURE);

  
  for (;;) {
    // Block until 20ms interval
    TickType_t xLastWakeTime = xTaskGetTickCount();
    vTaskDelayUntil(&xLastWakeTime, xFrequency);

    // taskENTER_CRITICAL();
    // baro->update(baro);
    // taskEXIT_CRITICAL();

    // Grab sensor data over CAN line.
    uint8_t buffer[7];
    while (xQueueReceive(CAN_Queue_Baro_Ground_Pressure.queue, &baro->groundPress, 0));
    while (xQueueReceive(CAN_Queue_Baro_Pressure.queue,        &buffer[0],         0)) {
      memcpy(&buffer[0], baro->rawPress, 3);
      memcpy(&buffer[3], (uint8_t*)&baro->press, 4);
    }
    while (xQueueReceive(CAN_Queue_Baro_Temperature.queue,     &buffer[0], 0)) {
      memcpy(&buffer[0], baro->rawTemp, 3);
      memcpy(&buffer[3], (uint8_t*)&baro->temp, 4);
    }
    
    // Calculate altitude
    state->altitude = 44330 * (1.0 - pow(baro->press / baro->groundPress, 0.1903));

    // Add sensor data and barometer data to dataframe
    state->mem.append(&state->mem, HEADER_LOWRES);
    state->mem.appendBytes(&state->mem, baro->rawTemp, baro->tempDataSize);
    state->mem.appendBytes(&state->mem, baro->rawPress, baro->pressDataSize);

    // Only run calculations when enabled
    EventBits_t uxBits = xEventGroupWaitBits(xTaskEnableGroup, GROUP_TASK_ENABLE_LOWRES, pdFALSE, pdFALSE, blockTime);
    if (uxBits & GROUP_TASK_ENABLE_LOWRES) {
      // Calculate state
      z.pData[0] = state->altitude;
      z.pData[1] = (state->cosine * 9.81 * accel->accelData[ZINDEX] - 9.81); // Acceleration measured in m/s^2
      kf.update(&kf, &z);

      state->velocity = kf.x.pData[1];
      state->avgPress.append(&state->avgPress, baro->press);
      state->avgVel.append(&state->avgVel, state->velocity);
    }
  }
}


void vLDataAcquisition_Secondary(void *argument) {
  float dt = 0.020;
  KalmanFilter kf;
  KalmanFilter_init(&kf);

  //! @todo Move kalman filter matrices into context struct
  // Initialise filter parameters
  float A[9] = {
    1.0, dt, 0.5 * (dt * dt),
    0.0, 1.0, dt,
    0.0, 0.0, 1.0
  };
  kf.A.pData = A;
  float Q[9] = {
    99.52, 0.0, 0.0,
    0.0, 1.42, 0.0,
    0.0, 0.0, 6.27
  };
  kf.Q.pData = Q;
  float R[4] = {
    97.92, 0.0,
    0.0, 0.61
  };
  kf.R.pData = R;
  float P[9] = {
    1, 0.0, 0.0,
    0.0, 0.1, 0.0,
    0.0, 0.0, 100.0
  };
  kf.P.pData = P;

  // Initialise measurement matrix
  arm_matrix_instance_f32 z;
  float zData[2] = {0.0, 0.0};
  arm_mat_init_f32(&z, 2, 1, zData);

  TickType_t xLastWakeTime;
  const TickType_t xFrequency = pdMS_TO_TICKS(20); // 50Hz
  const TickType_t blockTime  = pdMS_TO_TICKS(0);

  UART_t *usb                 = DeviceList_getDeviceHandle(DEVICE_UART_USB).device;
  Baro_t *baro                = DeviceList_getDeviceHandle(DEVICE_BARO).device;
  Accel_t *accel              = DeviceList_getDeviceHandle(DEVICE_ACCEL).device;

  State *state                = State_getState();

  
  CAN_Packet CAN_Packet_Baro_Ground_Pressure;
  CAN_Packet_Baro_Ground_Pressure.id = CAN_ID_BARO_GROUND_PRESSURE;
  CAN_Packet_Baro_Ground_Pressur.data.length = 4;
  
  CAN_Packet CAN_Packet_Baro_Pressure;
  CAN_Packet_Baro_Pressure.id = CAN_ID_BARO_PRESSURE;
  CAN_Packet_Baro_Pressure.data.length = 7;

  CAN_Packet CAN_Packet_Baro_Temperature;
  CAN_Packet_Baro_Temperature.id = CAN_ID_BARO_TEMPERATURE;
  CAN_Packet_Baro_Temperature.data.length = 7;


  for (;;) {
    // Block until 20ms interval
    TickType_t xLastWakeTime = xTaskGetTickCount();
    vTaskDelayUntil(&xLastWakeTime, xFrequency);

    taskENTER_CRITICAL();
    baro->update(baro);
    taskEXIT_CRITICAL();

    // Send sensor data over CAN.
    memcpy(&CAN_Packet_Baro_Ground_Pressure.data.byte, (uint8_t*)&baro->groundPress, 4);
    CAN_Transmission_Queue_Add(&CAN_Packet_Baro_Ground_Pressure);

    memcpy(&CAN_Packet_Baro_Pressure.data.byte[0], baro->rawPress, 3);
    memcpy(&CAN_Packet_Baro_Pressure.data.byte[3], (uint8_t*)&baro->press, 4);
    CAN_Transmission_Queue_Add(&CAN_Packet_Baro_Pressure);

    memcpy(&CAN_Packet_Baro_Temperature.data.byte[0], baro->rawTemp, 3);
    memcpy(&CAN_Packet_Baro_Temperature.data.byte[3], (uint8_t*)&baro->temp, 4);
    CAN_Transmission_Queue_Add(&CAN_Packet_Baro_Temperature);
    

    // Calculate altitude
    state->altitude = 44330 * (1.0 - pow(baro->press / baro->groundPress, 0.1903));

    // Add sensor data and barometer data to dataframe
    state->mem.append(&state->mem, HEADER_LOWRES);
    state->mem.appendBytes(&state->mem, baro->rawTemp, baro->tempDataSize);
    state->mem.appendBytes(&state->mem, baro->rawPress, baro->pressDataSize);

    // Only run calculations when enabled
    EventBits_t uxBits = xEventGroupWaitBits(xTaskEnableGroup, GROUP_TASK_ENABLE_LOWRES, pdFALSE, pdFALSE, blockTime);
    if (uxBits & GROUP_TASK_ENABLE_LOWRES) {
      // Calculate state
      z.pData[0] = state->altitude;
      z.pData[1] = (state->cosine * 9.81 * accel->accelData[ZINDEX] - 9.81); // Acceleration measured in m/s^2
      kf.update(&kf, &z);

      state->velocity = kf.x.pData[1];
      state->avgPress.append(&state->avgPress, baro->press);
      state->avgVel.append(&state->avgVel, state->velocity);
    }
  }
}
