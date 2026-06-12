/**
 * @author Matt Ricci
 * @ingroup Sensors
 * @addtogroup Barometer
 * @file BMP581.h
 * @todo Document interface
 */

#ifndef _BMP581_DUMMY_H
#define _BMP581_DUMMY_H

#include "bmp581.h"


BMP581_t BMP581_dummy_init(BMP581_t *, SPI_t *, GPIOpin_t, const float, const float);
void BMP581_dummy_update(Baro_t *);
void BMP581_dummy_readTemp(Baro_t *, float *);
void BMP581_dummy_readPress(Baro_t *, float *);
void BMP581_dummy_readRawTemp(Baro_t *, uint8_t *);
void BMP581_dummy_readRawPress(Baro_t *, uint8_t *);
void BMP581_dummy_processRawTemp(Baro_t *, uint8_t *, float *);
void BMP581_dummy_processRawPress(Baro_t *, uint8_t *, float *);

uint8_t BMP581_dummy_readRegister(BMP581_t *, uint8_t);
void BMP581_dummy_readRegisters(BMP581_t *, uint8_t, uint8_t, uint8_t *);
void BMP581_dummy_writeRegister(BMP581_t *, uint8_t, uint8_t);

/** @} */
#endif
