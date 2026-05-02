/***********************************************************************************
 * @file        iam_20380.c                                                        *
 * @author      Matt Ricci                                                         *
 * @addtogroup  IAM_20380                                                          *
 * @{                                                                              *
 ***********************************************************************************/

#include "iam_20380.h"

#include "string.h"

static uint8_t IAM_20380_readRegister(IAM_20380_t *, uint8_t);
static void IAM_20380_writeRegister(IAM_20380_t *, uint8_t, uint8_t);

/* =============================================================================== */
/**
 * @brief Initialiser for a IAM_20380 gyroscope.
 *
 * @param *gyro 			Pointer to IAM_20380 struct to be initialised.
 * @param *port 			Pointer to GPIO port struct.
 * @param cs 					Device chip select address.
 * @param scale       Selected scale for read gyro rates.
 * @param *axes       Array defining sensor mounting axes.
 * @return @c NULL.
 **
 * =============================================================================== */
IAM_20380_t IAM_20380_init(
  IAM_20380_t *gyro,
  SPI_t *spi,
  GPIOpin_t cs,
  float sensitivity,
  const uint8_t *axes,
  const int8_t *sign
) {
  gyro->spi                  = spi;
  gyro->cs                   = cs;
  gyro->base.sensitivity     = sensitivity;
  gyro->base.dataSize        = IAM_20380_DATA_TOTAL;
  gyro->base.update          = IAM_20380_update;
  gyro->base.readGyro        = IAM_20380_readGyro;
  gyro->base.readRawBytes    = IAM_20380_readRawBytes;
  gyro->base.processRawBytes = IAM_20380_processRawBytes;
  gyro->base.bias            = gyro->bias;
  gyro->base.axes            = gyro->axes;
  gyro->base.sign            = gyro->sign;
  gyro->base.gyroData        = gyro->gyroData;
  gyro->base.rawGyroData     = gyro->rawGyroData;
  memcpy(gyro->base.axes, axes, IAM_20380_DATA_COUNT);
  memcpy(gyro->base.sign, sign, IAM_20380_DATA_COUNT);

  // TODO:
  // Make scale & sensitivity for gyroscopes configurable on init.
  // See kx134_1211.c for example.

  // Reset chip and select clock source
  IAM_20380_writeRegister(
    gyro,
    IAM_20380_PWR_MGMT_1,
    (IAM_20380_PWR_MGMT_1_RESET     // Manual software reset
     | IAM_20380_PWR_MGMT_1_CLKSEL) // Auto clock select
  );

  // Apparently power-on reset delay also applies to soft resets.
  // There is not indication of this whatsoever in the datasheet.
  for (uint32_t i = 0; i < 0x1FFFF; i++);

  // Set output data rate to 1kHz
  IAM_20380_writeRegister(gyro, IAM_20380_SMPLRT_DIV, 0x07);

  // Set resolution to 500dps
  IAM_20380_writeRegister(gyro, IAM_20380_GYRO_CONFIG, IAM_20380_CONFIG_FS_SEL500);

  // Configure filter for minimal noise
  IAM_20380_writeRegister(gyro, IAM_20380_CONFIG, 6);

  return *gyro;
}

/******************************** DEVICE METHODS ********************************/

/* =============================================================================== */
/**
 * @brief Read 3-axis floating point gyro rates.
 *
 * @param 	*gyro 		Pointer to gyro struct.
 * @param 	*out 		  Floating point gyro rate array.
 * @returns @c NULL.
 **
 * =============================================================================== */
void IAM_20380_readGyro(Gyro_t *gyro, float *out) {
  uint8_t bytes[IAM_20380_DATA_TOTAL];
  gyro->readRawBytes(gyro, bytes);
  gyro->processRawBytes(gyro, bytes, out);
}

/* =============================================================================== */
/**
 * @brief Updates internally stored gyro readings.
 *
 * @param 	*gyro 		Pointer to gyro struct.
 * @returns @c NULL.
 **
 * =============================================================================== */
void IAM_20380_update(Gyro_t *gyro) {
  gyro->readRawBytes(gyro, gyro->rawGyroData);
  gyro->processRawBytes(gyro, gyro->rawGyroData, gyro->gyroData);
}

/* =============================================================================== */
/**
 * @brief Process raw 3-axis data to floating point gyro rates.
 *
 * @param 	*gyro 		Pointer to gyro struct.
 * @param 	*bytes 		Raw 3-axis data array.
 * @param 	*out 			Processed 3-axis data array to write.
 * @returns @c NULL.
 **
 * =============================================================================== */
void IAM_20380_processRawBytes(Gyro_t *gyro, uint8_t *bytes, float *out) {
  for (int i = 0; i < IAM_20380_DATA_COUNT; i++) {
    out[i] = (int16_t)(((uint16_t)bytes[i * 2] << 8) | bytes[i * 2 + 1]);  // Process bytes
    out[i] = (out[i] * gyro->sign[i] * gyro->sensitivity) - gyro->bias[i]; // Scale and offset
  }
}

/* =============================================================================== */
/**
 * @brief Read raw 3-axis data.
 *
 * @param 	*gyro 		Pointer to gyro struct.
 * @param 	*out 			Raw 3-axis data array to write.
 * @returns @c NULL.
 **
 * =============================================================================== */
void IAM_20380_readRawBytes(Gyro_t *gyro, uint8_t *out) {
#define INDEX_AXES(index, byte) 2 * gyro->axes[index] + byte
  out[INDEX_AXES(0, 0)] = IAM_20380_readRegister((IAM_20380_t *)gyro, IAM_20380_XOUT_H); // gyro X high
  out[INDEX_AXES(0, 1)] = IAM_20380_readRegister((IAM_20380_t *)gyro, IAM_20380_XOUT_L); // gyro X low
  out[INDEX_AXES(1, 0)] = IAM_20380_readRegister((IAM_20380_t *)gyro, IAM_20380_YOUT_H); // gyro Y high
  out[INDEX_AXES(1, 1)] = IAM_20380_readRegister((IAM_20380_t *)gyro, IAM_20380_YOUT_L); // gyro Y low
  out[INDEX_AXES(2, 0)] = IAM_20380_readRegister((IAM_20380_t *)gyro, IAM_20380_ZOUT_H); // gyro Z high
  out[INDEX_AXES(2, 1)] = IAM_20380_readRegister((IAM_20380_t *)gyro, IAM_20380_ZOUT_L); // gyro Z low
#undef INDEX_AXES
}

/******************************** INTERFACE METHODS ********************************/

void IAM_20380_writeRegister(IAM_20380_t *gyro, uint8_t address, uint8_t data) {
  SPI_t *spi   = gyro->spi;
  GPIOpin_t cs = gyro->cs;

  cs.reset(&cs);

  // Send read command and address
  uint8_t payload = address & 0x7F; // Load payload with address and read command
  spi->transmit(spi, payload);      // Transmit payload
  spi->transmit(spi, data);         // Transmit dummy data and read response data

  cs.set(&cs);
}

uint8_t IAM_20380_readRegister(IAM_20380_t *gyro, uint8_t address) {
  uint8_t response = 0;
  SPI_t *spi       = gyro->spi;
  GPIOpin_t cs     = gyro->cs;

  cs.reset(&cs);

  // Send read command and address
  uint8_t payload = address | 0x80;              // Load payload with address and read command
  response        = spi->transmit(spi, payload); // Transmit payload
  response        = spi->transmit(spi, 0xFF);    // Transmit dummy data and read response data

  cs.set(&cs);

  return response;
}

/** @} */
