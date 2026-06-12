/***********************************************************************************
 * @file        BMP581.c                                                           *
 * @author      Matt Ricci                                                         *
 * @addtogroup  BMP581                                                             *
 *                                                                                 *
 * @todo Add altitude calculation method                                           *
 * @todo Document implementation                                                   *
 * @todo Move private interface methods (read/write register) to static functions  *
 *       with internal prototypes.                                                 *
 * @todo Replace giga loops with hardware timer                                    *
 * @{                                                                              *
 ***********************************************************************************/

#include "bmp581_dummy.h"

BMP581_t BMP581_dummy_init(
  BMP581_t *baro,
  SPI_t *spi,
  GPIOpin_t cs,
  float tempSensitivity,
  float pressSensitivity
) {

  baro->spi                  = spi;
  baro->cs                   = cs;
  baro->tempSensitivity      = tempSensitivity;
  baro->base.sensitivity     = pressSensitivity;
  baro->base.tempDataSize    = BMP581_DATA_SIZE;
  baro->base.pressDataSize   = BMP581_DATA_SIZE;
  baro->base.update          = BMP581_dummy_update;
  baro->base.readTemp        = BMP581_dummy_readTemp;
  baro->base.readRawTemp     = BMP581_dummy_readRawTemp;
  baro->base.processRawTemp  = BMP581_dummy_processRawTemp;
  baro->base.readPress       = BMP581_dummy_readPress;
  baro->base.readRawPress    = BMP581_dummy_readRawPress;
  baro->base.processRawPress = BMP581_dummy_processRawPress;
  baro->base.rawPress        = baro->rawPress;
  baro->base.rawTemp         = baro->rawTemp;

  return *baro;
}

/******************************** DEVICE METHODS ********************************/

/* =============================================================================== */
/**
 * @brief Updates the BMP581 barometer readings.
 * @param *baro Pointer to BMP581 struct to be updated.
 * @returns @c NULL.
 **
 * =============================================================================== */
void BMP581_dummy_update(Baro_t *baro) {
}

/* =============================================================================== */
/**
 * @brief Read the temperature from the BMP581 sensor.
 * @param *baro Pointer to BMP581 struct.
 * @param *out Pointer to float where the temperature will be stored.
 * @returns @c NULL.
 **
 * =============================================================================== */
void BMP581_dummy_readTemp(Baro_t *baro, float *out) {
}

/* =============================================================================== */
/**
 * @brief Processes raw temperature data from BMP581 sensor.
 * @param *baro  Pointer to BMP581 struct.
 * @param *bytes Pointer to array containing raw temperature.
 * @param *out   Pointer to a float where processed temperature value will be stored.
 * @returns @c NULL.
 **
 * =============================================================================== */
void BMP581_dummy_processRawTemp(Baro_t *baro, uint8_t *bytes, float *out) {
}

/* =============================================================================== */
/**
 * @brief
 * @param
 * @param
 * @returns @c NULL.
 **
 * =============================================================================== */
void BMP581_dummy_readRawTemp(Baro_t *baro, uint8_t *out) {
}

/* =============================================================================== */
/**
 * @brief
 * @param
 * @param
 * @returns @c NULL.
 **
 * =============================================================================== */
void BMP581_dummy_readPress(Baro_t *baro, float *out) {
}

/* =============================================================================== */
/**
 * @brief
 * @param
 * @param
 * @returns @c NULL.
 **
 * =============================================================================== */
void BMP581_dummy_processRawPress(Baro_t *baro, uint8_t *bytes, float *out) {
}

/* =============================================================================== */
/**
 * @brief
 * @param
 * @param
 * @returns @c NULL.
 **
 * =============================================================================== */
void BMP581_dummy_readRawPress(Baro_t *baro, uint8_t *out) {
}

/******************************** INTERFACE METHODS ********************************/

void BMP581_dummy_writeRegister(BMP581_t *baro, uint8_t address, uint8_t data) {
}

uint8_t BMP581_dummy_readRegister(BMP581_t *baro, uint8_t address) {
  return 0x00;
}

void BMP581_dummy_readRegisters(BMP581_t *baro, uint8_t address, uint8_t count, uint8_t *out) {
}
