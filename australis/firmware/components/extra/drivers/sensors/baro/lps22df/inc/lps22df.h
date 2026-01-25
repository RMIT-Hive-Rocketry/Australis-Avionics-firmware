/**
 * @author Matt Ricci
 * @ingroup Sensors
 * @addtogroup Barometer
 * @file lps22df.h
 * @todo Document interface
 */

// ALLOW FORMATTING
#ifndef _LPS22DF_H
#define _LPS22DF_H

#include "barometer.h"

#include "spi.h"
#include "gpiopin.h"

#define LPS22DF_TEMP_SENSITIVITY  (1.0f / 100)
#define LPS22DF_PRESS_SENSITIVITY (100.0f / 4096)

#define LPS22DF_CTRL_REG1         0x10
#define LPS22DF_CTRL_REG2         0x11
#define LPS22DF_CTRL_REG3         0x12
#define LPS22DF_CTRL_REG4         0x13

#define LPS22DF_RPDS_L            0x1A
#define LPS22DF_RPDS_H            0x1B

#define LPS22DF_PRESS_OUT_XL      0x28
#define LPS22DF_PRESS_OUT_L       0x29
#define LPS22DF_PRESS_OUT_H       0x2A

#define LPS22DF_TEMP_OUT_L        0x2B
#define LPS22DF_TEMP_OUT_H        0x2C

#define LPS22DF_CTRL_REG1_ODR_OFF 0x00
#define LPS22DF_CTRL_REG1_ODR_1   0x08
#define LPS22DF_CTRL_REG1_ODR_4   0x10
#define LPS22DF_CTRL_REG1_ODR_10  0x18
#define LPS22DF_CTRL_REG1_ODR_25  0x20
#define LPS22DF_CTRL_REG1_ODR_50  0x28
#define LPS22DF_CTRL_REG1_ODR_75  0x30
#define LPS22DF_CTRL_REG1_ODR_100 0x38
#define LPS22DF_CTRL_REG1_ODR_200 0x40

#define LPS22DF_CTRL_REG1_AVG_4   0x00
#define LPS22DF_CTRL_REG1_AVG_8   0x01
#define LPS22DF_CTRL_REG1_AVG_16  0x02
#define LPS22DF_CTRL_REG1_AVG_32  0x03
#define LPS22DF_CTRL_REG1_AVG_64  0x04
#define LPS22DF_CTRL_REG1_AVG_128 0x05
#define LPS22DF_CTRL_REG1_AVG_512 0x07

#define LPS22DF_CTRL_REG2_BDU     0x08
#define LPS22DF_CTRL_REG2_SWRESET 0x04

#define LPS22DF_PRESS_DATA_SIZE   3 // Three bytes per pressure reading
#define LPS22DF_TEMP_DATA_SIZE    2 // Two bytes per temp reading
#define LPS22DF_DATA_COUNT        2 // Two readings - temperature, pressure
#define LPS22DF_DATA_TOTAL        (LPS22DF_TEMP_DATA_SIZE + LPS22DF_PRESS_DATA_SIZE)

/**
 * @ingroup Barometer
 * @addtogroup LPS22DF
 * @{
 */

/** @extends SPI */
typedef struct LPS22DF {
  Baro_t base;
  SPI_t *spi;
  GPIOpin_t cs;
  float pressSensitivity;
  float tempSensitivity;
  uint8_t rawTemp[LPS22DF_PRESS_DATA_SIZE];
  uint8_t rawPress[LPS22DF_TEMP_DATA_SIZE];
} LPS22DF_t;

LPS22DF_t LPS22DF_init(LPS22DF_t *, SPI_t *, GPIOpin_t, const float, const float);
void LPS22DF_update(Baro_t *);
void LPS22DF_readTemp(Baro_t *, float *);
void LPS22DF_readPress(Baro_t *, float *);
void LPS22DF_readRawTemp(Baro_t *, uint8_t *);
void LPS22DF_readRawPress(Baro_t *, uint8_t *);
void LPS22DF_processRawTemp(Baro_t *, uint8_t *, float *);
void LPS22DF_processRawPress(Baro_t *, uint8_t *, float *);

/** @} */
#endif
