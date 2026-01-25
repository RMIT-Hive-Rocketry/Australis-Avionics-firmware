/***********************************************************************************
 * @file        A3G4250D.c                                                         *
 * @author      Matt Ricci                                                         *
 * @addtogroup  A3G4250D                                                           *
 *                                                                                 *
 * @todo Move private interface methods (read/write register) to static functions  *
 *       with internal prototypes.                                                 *
 * @{                                                                              *
 ***********************************************************************************/

#include "a3g4250d.h"

#include "string.h"

static uint8_t A3G4250D_readRegister(A3G4250D_t *, uint8_t);
static void A3G4250D_writeRegister(A3G4250D_t *, uint8_t, uint8_t);

/* =============================================================================== */
/**
 * @brief Initialiser for a A3G4250D gyroscope.
 *
 * @param *gyro 			Pointer to A3G4250D struct to be initialised.
 * @param *port 			Pointer to GPIO port struct.
 * @param cs 					Device chip select address.
 * @param scale       Selected scale for read gyro rates.
 * @param *axes       Array defining sensor mounting axes.
 * @return @c NULL.
 **
 * =============================================================================== */
A3G4250D_t A3G4250D_init(
  A3G4250D_t *gyro,
  SPI_t *spi,
  GPIOpin_t cs,
  float sensitivity,
  const uint8_t *axes,
  const int8_t *sign
) {
  gyro->spi                  = spi;
  gyro->cs                   = cs;
  gyro->base.sensitivity     = sensitivity;
  gyro->base.dataSize        = A3G4250D_DATA_TOTAL;
  gyro->base.update          = A3G4250D_update;
  gyro->base.readGyro        = A3G4250D_readGyro;
  gyro->base.readRawBytes    = A3G4250D_readRawBytes;
  gyro->base.processRawBytes = A3G4250D_processRawBytes;
  gyro->base.bias            = gyro->bias;
  gyro->base.axes            = gyro->axes;
  gyro->base.sign            = gyro->sign;
  gyro->base.gyroData        = gyro->gyroData;
  gyro->base.rawGyroData     = gyro->rawGyroData;
  memcpy(gyro->base.axes, axes, A3G4250D_DATA_COUNT);
  memcpy(gyro->base.sign, sign, A3G4250D_DATA_COUNT);

  // TODO:
  // Make scale & sensitivity for gyroscopes configurable on init.
  // See kx134_1211.c for example.

  A3G4250D_writeRegister(
    gyro,
    A3G4250D_CTRL_REG1,
    (A3G4250D_CTRL_REG1_ODR_800Hz     // Set data rate to 800Hz
     | A3G4250D_CTRL_REG1_AXIS_ENABLE // Enable axis measurement
     | A3G4250D_CTRL_REG1_PD_ENABLE)  // Set power mode to normal
  );

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
void A3G4250D_readGyro(Gyro_t *gyro, float *out) {
  uint8_t bytes[A3G4250D_DATA_TOTAL];
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
void A3G4250D_update(Gyro_t *gyro) {
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
void A3G4250D_processRawBytes(Gyro_t *gyro, uint8_t *bytes, float *out) {
  for (int i = 0; i < A3G4250D_DATA_COUNT; i++) {
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
void A3G4250D_readRawBytes(Gyro_t *gyro, uint8_t *out) {
#define INDEX_AXES(index, byte) 2 * gyro->axes[index] + byte
  out[INDEX_AXES(0, 0)] = A3G4250D_readRegister((A3G4250D_t *)gyro, A3G4250D_OUT_X_H); // gyro X high
  out[INDEX_AXES(0, 1)] = A3G4250D_readRegister((A3G4250D_t *)gyro, A3G4250D_OUT_X_L); // gyro X low
  out[INDEX_AXES(1, 0)] = A3G4250D_readRegister((A3G4250D_t *)gyro, A3G4250D_OUT_Y_H); // gyro Y high
  out[INDEX_AXES(1, 1)] = A3G4250D_readRegister((A3G4250D_t *)gyro, A3G4250D_OUT_Y_L); // gyro Y low
  out[INDEX_AXES(2, 0)] = A3G4250D_readRegister((A3G4250D_t *)gyro, A3G4250D_OUT_Z_H); // gyro Z high
  out[INDEX_AXES(2, 1)] = A3G4250D_readRegister((A3G4250D_t *)gyro, A3G4250D_OUT_Z_L); // gyro Z low
#undef INDEX_AXES
}

/******************************** INTERFACE METHODS ********************************/

// @TODO: Implement burst read function readRegisters and update A3G4250D_readRawBytes

void A3G4250D_writeRegister(A3G4250D_t *gyro, uint8_t address, uint8_t data) {
  SPI_t *spi   = gyro->spi;
  GPIOpin_t cs = gyro->cs;

  cs.reset(&cs);

  // Send read command and address
  uint8_t payload = address & 0x7F; // Load payload with address and read command
  spi->transmit(spi, payload);      // Transmit payload
  spi->transmit(spi, data);         // Transmit dummy data and read response data

  cs.set(&cs);
}

uint8_t A3G4250D_readRegister(A3G4250D_t *gyro, uint8_t address) {
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
