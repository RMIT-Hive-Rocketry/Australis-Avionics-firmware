/***********************************************************************************
 * @file        lps22df.c                                                          *
 * @author      Matt Ricci                                                         *
 * @addtogroup  LPS22DF                                                            *
 *                                                                                 *
 * @todo Add altitude calculation method                                           *
 * @{                                                                              *
 ***********************************************************************************/

#include "lps22df.h"

static uint8_t LPS22DF_readRegister(LPS22DF_t *, uint8_t);
static void LPS22DF_readRegisters(LPS22DF_t *, uint8_t, uint8_t, uint8_t *);
static void LPS22DF_writeRegister(LPS22DF_t *, uint8_t, uint8_t);

/* =============================================================================== */
/**
 * @brief Initialiser for a LPS22DF barometer.
 * @param *baro 						Pointer to LPS22DF struct to be initialised.
 * @param *port 						Pointer to GPIO port struct.
 * @param cs 								Device chip select address.
 * @param tempSensitivity   Barometer temperature sensitivity.
 * @param pressSensitivity  Barometer pressure sensitivity.
 * @return @c NULL.
 **
 * =============================================================================== */
LPS22DF_t LPS22DF_init(
  LPS22DF_t *baro,
  SPI_t *spi,
  GPIOpin_t cs,
  float tempSensitivity,
  float pressSensitivity
) {
  baro->spi                  = spi;
  baro->cs                   = cs;
  baro->tempSensitivity      = tempSensitivity;
  baro->base.sensitivity     = pressSensitivity;
  baro->base.pressDataSize   = LPS22DF_PRESS_DATA_SIZE;
  baro->base.tempDataSize    = LPS22DF_TEMP_DATA_SIZE;
  baro->base.update          = LPS22DF_update;
  baro->base.readTemp        = LPS22DF_readTemp;
  baro->base.readRawTemp     = LPS22DF_readRawTemp;
  baro->base.processRawTemp  = LPS22DF_processRawTemp;
  baro->base.readPress       = LPS22DF_readPress;
  baro->base.readRawPress    = LPS22DF_readRawPress;
  baro->base.processRawPress = LPS22DF_processRawPress;
  baro->base.rawPress        = baro->rawPress;
  baro->base.rawTemp         = baro->rawTemp;

  // Enable block data update
  LPS22DF_writeRegister(baro, LPS22DF_CTRL_REG2, LPS22DF_CTRL_REG2_BDU);

  // Enable continuous data with oversampling
  LPS22DF_writeRegister(
    baro,
    LPS22DF_CTRL_REG1,
    (LPS22DF_CTRL_REG1_ODR_200   // Continuous read 200Hz
     | LPS22DF_CTRL_REG1_AVG_32) // 32x oversample
  );

  // Set ground pressure reading on init
  for (uint32_t i = 0; i < 0x1FFFF; i++);                        // Wait for at least t_reconf
  baro->base.readPress((Baro_t *)baro, &baro->base.groundPress); // Read current pressure

  return *baro;
}

/******************************** DEVICE METHODS ********************************/

/* =============================================================================== */
/**
 * @brief Updates the LPS22DF barometer readings.
 * @param *baro Pointer to LPS22DF struct to be updated.
 * @returns @c NULL.
 **
 * =============================================================================== */
void LPS22DF_update(Baro_t *baro) {
  baro->readRawTemp(baro, baro->rawTemp);
  baro->processRawTemp(baro, baro->rawTemp, &baro->temp);

  baro->readRawPress(baro, baro->rawPress);
  baro->processRawPress(baro, baro->rawPress, &baro->press);
}

/* =============================================================================== */
/**
 * @brief Read the temperature from the LPS22DF sensor.
 * @param *baro Pointer to LPS22DF struct.
 * @param *out Pointer to float where the temperature will be stored.
 * @returns @c NULL.
 **
 * =============================================================================== */
void LPS22DF_readTemp(Baro_t *baro, float *out) {
  uint8_t bytes[LPS22DF_TEMP_DATA_SIZE];
  baro->readRawTemp(baro, bytes);
  baro->processRawTemp(baro, bytes, out);
}

/* =============================================================================== */
/**
 * @brief Processes raw temperature data from LPS22DF sensor.
 * @param *baro  Pointer to LPS22DF struct.
 * @param *bytes Pointer to array containing raw temperature.
 * @param *out   Pointer to a float where processed temperature value will be stored.
 * @returns @c NULL.
 **
 * =============================================================================== */
void LPS22DF_processRawTemp(Baro_t *baro, uint8_t *bytes, float *out) {
  *out = ((LPS22DF_t *)baro)->tempSensitivity * (int16_t)(((uint16_t)bytes[1] << 8) | bytes[0]);
}

/* =============================================================================== */
/**
 * @brief
 * @param
 * @param
 * @returns @c NULL.
 **
 * =============================================================================== */
void LPS22DF_readRawTemp(Baro_t *baro, uint8_t *out) {
  out[0] = LPS22DF_readRegister(((LPS22DF_t *)baro), LPS22DF_TEMP_OUT_H); // temp high
  out[1] = LPS22DF_readRegister(((LPS22DF_t *)baro), LPS22DF_TEMP_OUT_L); // temp low
}

/* =============================================================================== */
/**
 * @brief
 * @param
 * @param
 * @returns @c NULL.
 **
 * =============================================================================== */
void LPS22DF_readPress(Baro_t *baro, float *out) {
  uint8_t bytes[LPS22DF_PRESS_DATA_SIZE];
  baro->readRawPress(baro, bytes);
  baro->processRawPress(baro, bytes, out);
}

/* =============================================================================== */
/**
 * @brief
 * @param
 * @param
 * @returns @c NULL.
 **
 * =============================================================================== */
void LPS22DF_processRawPress(Baro_t *baro, uint8_t *bytes, float *out) {
  *out = ((LPS22DF_t *)baro)->base.sensitivity * (int32_t)(((uint32_t)bytes[0] << 16) | ((uint32_t)bytes[1] << 8) | bytes[0]);
}

/* =============================================================================== */
/**
 * @brief
 * @param
 * @param
 * @returns @c NULL.
 **
 * =============================================================================== */
void LPS22DF_readRawPress(Baro_t *baro, uint8_t *out) {
  uint8_t tmp[LPS22DF_PRESS_DATA_SIZE];
  LPS22DF_readRegisters(((LPS22DF_t *)baro), LPS22DF_PRESS_OUT_XL, LPS22DF_PRESS_DATA_SIZE, tmp);
  out[0] = tmp[2]; // temp high
  out[1] = tmp[1]; // temp low
  out[2] = tmp[0]; // temp mid
}

/******************************** INTERFACE METHODS ********************************/

void LPS22DF_writeRegister(LPS22DF_t *baro, uint8_t address, uint8_t data) {
  SPI_t *spi   = baro->spi;
  GPIOpin_t cs = baro->cs;

  cs.reset(&cs);

  // Send read command and address
  uint8_t payload = address & 0x7F; // Load payload with address and read command
  spi->transmit(spi, payload);      // Transmit payload
  spi->transmit(spi, data);         // Transmit dummy data and read response data

  cs.set(&cs);
}

uint8_t LPS22DF_readRegister(LPS22DF_t *baro, uint8_t address) {
  uint8_t response = 0;
  SPI_t *spi       = baro->spi;
  GPIOpin_t cs     = baro->cs;

  cs.reset(&cs);

  // Send read command and address
  uint8_t payload = address | 0x80;              // Load payload with address and read command
  response        = spi->transmit(spi, payload); // Transmit payload
  response        = spi->transmit(spi, 0xFF);    // Transmit dummy data and read response data

  cs.set(&cs);

  return response;
}

void LPS22DF_readRegisters(LPS22DF_t *baro, uint8_t address, uint8_t count, uint8_t *out) {
  SPI_t *spi   = baro->spi;
  GPIOpin_t cs = baro->cs;

  cs.reset(&cs);

  // Send read command and address
  uint8_t payload = address | 0x80; // Load payload with address and read command
  spi->transmit(spi, payload);      // Transmit payload

  // Auto increment read through registers
  for (uint8_t i = 0; i < count; i++) {
    out[i] = spi->transmit(spi, 0xFF);
  }

  cs.set(&cs);
}
