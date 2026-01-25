/**
 * @author Matt Ricci
 * @ingroup Sensors
 * @addtogroup Gyroscope
 * @file IAM_20380.h
 */

// ALLOW FORMATTING
#ifndef _IAM_20380_H
#define _IAM_20380_H

#include "gyroscope.h"

#include "spi.h"
#include "gpiopin.h"

#define IAM_20380_SENSITIVITY_FS_SEL250  (1.0f / 131.0f)
#define IAM_20380_SENSITIVITY_FS_SEL500  (1.0f / 65.5f)
#define IAM_20380_SENSITIVITY_FS_SEL1000 (1.0f / 32.8f)
#define IAM_20380_SENSITIVITY_FS_SEL2000 (1.0f / 16.4f)
#define IAM_20380_SAMPLE_RATE_DIV(rate)  (1000.0f / rate) - 1

#define IAM_20380_WHO_AM_I               0x75

#define IAM_20380_XG_OFFS_USRH           0x13
#define IAM_20380_XG_OFFS_USRL           0x14
#define IAM_20380_YG_OFFS_USRH           0x15
#define IAM_20380_YG_OFFS_USRL           0x16
#define IAM_20380_ZG_OFFS_USRH           0x17
#define IAM_20380_ZG_OFFS_USRL           0x18

#define IAM_20380_SMPLRT_DIV             0x19
#define IAM_20380_CONFIG                 0x1A
#define IAM_20380_GYRO_CONFIG            0x1B

#define IAM_20380_XOUT_H                 0x43
#define IAM_20380_XOUT_L                 0x44
#define IAM_20380_YOUT_H                 0x45
#define IAM_20380_YOUT_L                 0x46
#define IAM_20380_ZOUT_H                 0x47
#define IAM_20380_ZOUT_L                 0x48

#define IAM_20380_CONFIG_FS_SEL250       0x00
#define IAM_20380_CONFIG_FS_SEL500       0x08
#define IAM_20380_CONFIG_FS_SEL1000      0x10
#define IAM_20380_CONFIG_FS_SEL2000      0x18

#define IAM_20380_USER_CTRL              0x6A
#define IAM_20380_PWR_MGMT_1             0x6B
#define IAM_20380_PWR_MGMT_2             0x6C

#define IAM_20380_PWR_MGMT_1_RESET       0x80
#define IAM_20380_PWR_MGMT_1_CLKSEL      0x01

#define IAM_20380_PWR_MGMT_2_STBY_XG     0x4
#define IAM_20380_PWR_MGMT_2_STBY_YG     0x2
#define IAM_20380_PWR_MGMT_2_STBY_ZG     0x1

#define IAM_20380_DATA_SIZE              2 // Two bytes per axis
#define IAM_20380_DATA_COUNT             3 // Three axes - X Y Z
#define IAM_20380_DATA_TOTAL             (IAM_20380_DATA_COUNT * IAM_20380_DATA_SIZE)

/**
 * @ingroup Gyroscope
 * @addtogroup IAM_20380
 * @{
 */

/** @extends SPI */
typedef struct IAM_20380 {
  Gyro_t base;                               //!< Base gyroscope API
  SPI_t *spi;                                //!< Parent SPI interface
  GPIOpin_t cs;                              //!< Chip select GPIO.
  uint8_t dataSize;                          //!< Total data size.
  float sensitivity;                         //!< Gyroscope sensitivity
  uint8_t axes[IAM_20380_DATA_COUNT];        //!< Array defining axes of mounting
  int8_t sign[IAM_20380_DATA_COUNT];         //!< Array defining sign of axes
  uint8_t rawGyroData[IAM_20380_DATA_TOTAL]; //!< Raw gyro rates array
  float gyroData[IAM_20380_DATA_COUNT];      //!< Processed gyro rates array
  float bias[IAM_20380_DATA_COUNT];          //!< Bias offset array
} IAM_20380_t;

IAM_20380_t IAM_20380_init(IAM_20380_t *, SPI_t *, GPIOpin_t, const float, const uint8_t *, const int8_t *);
void IAM_20380_update(Gyro_t *);
void IAM_20380_readGyro(Gyro_t *, float *);
void IAM_20380_readRawBytes(Gyro_t *, uint8_t *);
void IAM_20380_processRawBytes(Gyro_t *, uint8_t *, float *);

/** @} */
#endif
