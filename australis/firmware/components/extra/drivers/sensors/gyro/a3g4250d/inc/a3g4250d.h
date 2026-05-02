/**
 * @author Matt Ricci
 * @ingroup Sensors
 * @addtogroup Gyroscope
 * @file A3G4250D.h
 */

// ALLOW FORMATTING
#ifndef _A3G4250D_H
#define _A3G4250D_H

#include "gyroscope.h"

#include "spi.h"
#include "gpiopin.h"

#define A3G4250D_SENSITIVITY           (0.00875f)
#define A3G4250D_CTRL_REG1             0x20
#define A3G4250D_CTRL_REG1_ODR_800Hz   0xC0
#define A3G4250D_CTRL_REG1_PD_ENABLE   0x08
#define A3G4250D_CTRL_REG1_AXIS_ENABLE 0x07
#define A3G4250D_OUT_X_L               0x28
#define A3G4250D_OUT_X_H               0x29
#define A3G4250D_OUT_Y_L               0x2A
#define A3G4250D_OUT_Y_H               0x2B
#define A3G4250D_OUT_Z_L               0x2C
#define A3G4250D_OUT_Z_H               0x2D

#define A3G4250D_DATA_SIZE             2 // Two bytes per axis
#define A3G4250D_DATA_COUNT            3 // Three axes - X Y Z
#define A3G4250D_DATA_TOTAL            (A3G4250D_DATA_COUNT * A3G4250D_DATA_SIZE)

/**
 * @ingroup Gyroscope
 * @addtogroup A3G4250D
 * @{
 */

/** @extends SPI */
typedef struct A3G4250D {
  Gyro_t base;                              //!< Base gyroscope API
  SPI_t *spi;                               //!< Parent SPI interface
  GPIOpin_t cs;                             //!< Chip select GPIO.
  float sensitivity;                        //!< Gyroscope sensitivity
  uint8_t axes[A3G4250D_DATA_COUNT];        //!< Array defining axes of mounting
  int8_t sign[A3G4250D_DATA_COUNT];         //!< Array defining sign of axes
  uint8_t rawGyroData[A3G4250D_DATA_TOTAL]; //!< Raw rotation array
  float gyroData[A3G4250D_DATA_COUNT];      //!< Processed rotation array
  float bias[A3G4250D_DATA_COUNT];          //!< Bias offset array
} A3G4250D_t;

A3G4250D_t A3G4250D_init(A3G4250D_t *, SPI_t *, GPIOpin_t, const float, const uint8_t *, const int8_t *);
void A3G4250D_update(Gyro_t *);
void A3G4250D_readGyro(Gyro_t *, float *);
void A3G4250D_readRawBytes(Gyro_t *, uint8_t *);
void A3G4250D_processRawBytes(Gyro_t *, uint8_t *, float *);

/** @} */
#endif
