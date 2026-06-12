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

#include "bmp581.h"

/* =============================================================================== */
/**
 * @brief Initialiser for a BMP581 barometer.
 * @param *baro 						Pointer to BMP581 struct to be initialised.
 * @param *port 						Pointer to GPIO port struct.
 * @param cs 								Device chip select address.
 * @param tempSensitivity   Barometer temperature sensitivity.
 * @param pressSensitivity  Barometer pressure sensitivity.
 * @return @c NULL.
 **
 * =============================================================================== */
BMP581_t BMP581_init(
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
  baro->base.update          = BMP581_update;
  baro->base.readTemp        = BMP581_readTemp;
  baro->base.readRawTemp     = BMP581_readRawTemp;
  baro->base.processRawTemp  = BMP581_processRawTemp;
  baro->base.readPress       = BMP581_readPress;
  baro->base.readRawPress    = BMP581_readRawPress;
  baro->base.processRawPress = BMP581_processRawPress;
  baro->base.rawPress        = baro->rawPress;
  baro->base.rawTemp         = baro->rawTemp;

  // Initial dummy read
  BMP581_readRegister(baro, 0x01);

  // Soft reset device
  BMP581_writeRegister(baro, BMP581_CMD, 0xB6);

  while (BMP581_readRegister(baro, BMP581_CHIP_ID) == 0x00);                                           // Check chip ID
  while (BMP581_readRegister(baro, BMP581_INT_STATUS) != 0x10);                                        // Wait for POR complete
  while (!(BMP581_readRegister(baro, BMP581_STATUS) & BMP581_STATUS_NVM_RDY));                         // Check device status NVM ready
  while ((BMP581_readRegister(baro, BMP581_STATUS) & BMP581_STATUS_NVM_ERR));                          // Check device status NVM err

  BMP581_writeRegister(baro, BMP581_ODR_CFG, BMP581_ODR_CFG_DEEP_DIS);                                 // Disable deep sleep
  for (uint32_t i = 0; i < 0x1FFFF; i++);                                                              // Wait for at least t_standby
  BMP581_writeRegister(baro, BMP581_ODR_CFG, BMP581_ODR_CFG_DEEP_DIS | BMP581_ODR_CFG_PWR_CONTINUOUS); // Set continuous sample

  // Read OSR config for reserved bits and enable pressure measurement with 16x oversampling
  uint8_t OSRCFG = BMP581_readRegister(baro, BMP581_OSR_CFG);
  BMP581_writeRegister(baro, BMP581_OSR_CFG, (BMP581_OSR_CFG_RESERVED & OSRCFG) | BMP581_OSR_CFG_PRESS_EN | BMP581_OSR_CFG_OSR_P_16);

  // Set ground pressure reading on init
  for (uint32_t i = 0; i < 0x1FFFF; i++);                        // Wait for at least t_reconf
  baro->base.readPress((Baro_t *)baro, &baro->base.groundPress); // Read current pressure

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
void BMP581_update(Baro_t *baro) {
  baro->readRawTemp(baro, baro->rawTemp);
  baro->processRawTemp(baro, baro->rawTemp, &baro->temp);

  baro->readRawPress(baro, baro->rawPress);
  baro->processRawPress(baro, baro->rawPress, &baro->press);
}

/* =============================================================================== */
/**
 * @brief Read the temperature from the BMP581 sensor.
 * @param *baro Pointer to BMP581 struct.
 * @param *out Pointer to float where the temperature will be stored.
 * @returns @c NULL.
 **
 * =============================================================================== */
void BMP581_readTemp(Baro_t *baro, float *out) {
  uint8_t bytes[BMP581_DATA_TOTAL];
  baro->readRawTemp(baro, bytes);
  baro->processRawTemp(baro, bytes, out);
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
void BMP581_processRawTemp(Baro_t *baro, uint8_t *bytes, float *out) {
  *out = ((BMP581_t *)baro)->tempSensitivity * (int32_t)(((uint32_t)bytes[0] << 16) | ((uint32_t)bytes[1] << 8) | bytes[0]);
}

/* =============================================================================== */
/**
 * @brief
 * @param
 * @param
 * @returns @c NULL.
 **
 * =============================================================================== */
void BMP581_readRawTemp(Baro_t *baro, uint8_t *out) {
  out[0] = BMP581_readRegister((BMP581_t *)baro, BMP581_TEMPERATURE_MSB);  // temp high
  out[1] = BMP581_readRegister((BMP581_t *)baro, BMP581_TEMPERATURE_LSB);  // temp low
  out[2] = BMP581_readRegister((BMP581_t *)baro, BMP581_TEMPERATURE_XLSB); // temp mid
}

/* =============================================================================== */
/**
 * @brief
 * @param
 * @param
 * @returns @c NULL.
 **
 * =============================================================================== */
void BMP581_readPress(Baro_t *baro, float *out) {
  uint8_t bytes[BMP581_DATA_TOTAL];
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
void BMP581_processRawPress(Baro_t *baro, uint8_t *bytes, float *out) {
  *out = ((BMP581_t *)baro)->base.sensitivity * (int32_t)(((uint32_t)bytes[0] << 16) | ((uint32_t)bytes[1] << 8) | bytes[0]);
}

/* =============================================================================== */
/**
 * @brief
 * @param
 * @param
 * @returns @c NULL.
 **
 * =============================================================================== */
void BMP581_readRawPress(Baro_t *baro, uint8_t *out) {
  uint8_t tmp[BMP581_DATA_SIZE];
  BMP581_readRegisters((BMP581_t *)baro, BMP581_PRESSURE_XLSB, BMP581_DATA_SIZE, tmp);
  out[0] = tmp[2]; // temp high
  out[1] = tmp[1]; // temp low
  out[2] = tmp[0]; // temp mid
}

/******************************** INTERFACE METHODS ********************************/

void BMP581_writeRegister(BMP581_t *baro, uint8_t address, uint8_t data) {
  SPI_t *spi   = baro->spi;
  GPIOpin_t cs = baro->cs;

  cs.reset(&cs);

  // Send read command and address
  uint8_t payload = address & 0x7F; // Load payload with address and read command
  spi->transmit(spi, payload);      // Transmit payload
  spi->transmit(spi, data);         // Transmit dummy data and read response data

  cs.set(&cs);
}

uint8_t BMP581_readRegister(BMP581_t *baro, uint8_t address) {
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

void BMP581_readRegisters(BMP581_t *baro, uint8_t address, uint8_t count, uint8_t *out) {
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
