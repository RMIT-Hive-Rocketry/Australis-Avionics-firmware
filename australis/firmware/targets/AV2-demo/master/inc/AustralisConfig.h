// ALLOW FORMATTING
#ifndef AUSTRALISCONFIG_H
#define AUSTRALISCONFIG_H

#define CORE_MAX_SUBSCRIBERS

#define LORA_MSG_LENGTH            32
#define UART_MSG_LENGTH            255
#define coreTASK_ENABLE            1
#define coreFLASH_ENABLE           0
#define coreUSE_TRACE              0
#define targetENABLE_CONSOLE_PRINT 0

/* ===================================================================== *
 *                             FLIGHT THRESHOLDS                         *
 * ===================================================================== */

#define MAIN_ALTITUDE_METERS       390.0f
#define ACCEL_LAUNCH               3.0f

/* ===================================================================== *
 *                                DEVICE AXES                            *
 * ===================================================================== */

// Accel 1
#define ACCEL_AXES_1               ((const uint8_t[]){2, 1, 0})
#define ACCEL_SIGN_1               ((const int8_t[]){1, 1, 1})

// Accel 2
#define ACCEL_AXES_2               ((const uint8_t[]){2, 1, 0})
#define ACCEL_SIGN_2               ((const int8_t[]){1, 1, 1})

// Gyroscope
#define GYRO_AXES                  ((const uint8_t[]){2, 1, 0})
#define GYRO_SIGN                  ((const int8_t[]){1, 1, 1})

/* ===================================================================== *
 *                               DEVICE SCALE                            *
 * ===================================================================== */

#define ACCEL_SCALE_HIGH           32
#define ACCEL_SCALE_LOW            16

#endif
