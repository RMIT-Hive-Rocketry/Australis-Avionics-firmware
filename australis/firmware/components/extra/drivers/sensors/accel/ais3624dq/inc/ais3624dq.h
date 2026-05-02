/**
 * @author Matt Ricci
 * @ingroup Sensors
 * @addtogroup Accelerometer
 * @file ais3624dq.h
 */

// ALLOW FORMATTING
#ifndef AIS3624DQ_H
#define AIS3624DQ_H

#include "accelerometer.h"

#include "spi.h"
#include "gpiopin.h"

#define AIS3624DQ_SENSITIVITY_24G     (0.0117f / 16.0f)
#define AIS3624DQ_SENSITIVITY_12G     (0.0059f / 16.0f)
#define AIS3624DQ_SENSITIVITY_6G      (0.0029f / 16.0f)
#define AIS3624DQ_SENSITIVITY(scale)  AIS3624DQ_SENSITIVITY_##scale##G

#define AIS3624DQ_CTRL_REG1           0x20
#define AIS3624DQ_CTRL_REG2           0x21
#define AIS3624DQ_CTRL_REG3           0x22
#define AIS3624DQ_CTRL_REG4           0x23
#define AIS3624DQ_CTRL_REG5           0x24

#define AIS3624DQ_XOUT_L              0x28
#define AIS3624DQ_XOUT_H              0x29
#define AIS3624DQ_YOUT_L              0x2A
#define AIS3624DQ_YOUT_H              0x2B
#define AIS3624DQ_ZOUT_L              0x2C
#define AIS3624DQ_ZOUT_H              0x2D

#define AIS3624DQ_CTRL_REG1_PM_OFF    0x00
#define AIS3624DQ_CTRL_REG1_PM_NORMAL 0x20
#define AIS3624DQ_CTRL_REG1_DR_50     0x00
#define AIS3624DQ_CTRL_REG1_DR_100    0x08
#define AIS3624DQ_CTRL_REG1_DR_400    0x10
#define AIS3624DQ_CTRL_REG1_DR_1000   0x18
#define AIS3624DQ_CTRL_REG1_XEN       0x01
#define AIS3624DQ_CTRL_REG1_YEN       0x02
#define AIS3624DQ_CTRL_REG1_ZEN       0x04

#define AIS3624DQ_CTRL_REG4_BDU       0x80
#define AIS3624DQ_CTRL_REG4_FS_24G    0x30
#define AIS3624DQ_CTRL_REG4_FS_12G    0x10
#define AIS3624DQ_CTRL_REG4_FS_6G     0x00
#define AIS3624DQ_CTRL_REG4_FS(scale) AIS3624DQ_CTRL_REG4_FS_##scale##G

#define AIS3624DQ_DATA_SIZE           2 // Two bytes per axis
#define AIS3624DQ_DATA_COUNT          3 // Three axes - X Y Z
#define AIS3624DQ_DATA_TOTAL          (AIS3624DQ_DATA_COUNT * AIS3624DQ_DATA_SIZE)

/**
 * @ingroup Accelerometer
 * @defgroup AIS3624DQ
 * @addtogroup AIS3624DQ
 * @{
 */

/** @extends SPI */
typedef struct AIS3624DQ {
  Accel_t base;                               //!< Base accelerometer API
  SPI_t *spi;                                 //!< Parent SPI interface
  GPIOpin_t cs;                               //!< Chip select GPIO.
  float sensitivity;                          //!< Accelerometer sensitivity
  uint8_t axes[AIS3624DQ_DATA_COUNT];         //!< Array defining axes of mounting
  int8_t sign[AIS3624DQ_DATA_COUNT];          //!< Array defining sign of axes
  uint8_t rawAccelData[AIS3624DQ_DATA_TOTAL]; //!< Raw accelerations array
  float accelData[AIS3624DQ_DATA_COUNT];      //!< Processed accelerations array
} AIS3624DQ_t;

AIS3624DQ_t AIS3624DQ_init(AIS3624DQ_t *, SPI_t *, GPIOpin_t, const uint8_t, const uint8_t *, const int8_t *);
void AIS3624DQ_update(Accel_t *);
void AIS3624DQ_readAccel(Accel_t *, float *);
void AIS3624DQ_readRawBytes(Accel_t *, uint8_t *);
void AIS3624DQ_processRawBytes(Accel_t *, uint8_t *, float *);

/** @} */
#endif
