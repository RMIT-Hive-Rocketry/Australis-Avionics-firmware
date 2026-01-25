/***********************************************************************************
 * @file        KX134_1211.c                                                       *
 * @author      Matt Ricci                                                         *
 * @addtogroup  Accelerometer_KX134_1211                                           *
 *                                                                                 *
 * @todo Move private interface methods (read/write register) to static functions  *
 *       with internal prototypes.                                                 *
 * @{                                                                              *
 ***********************************************************************************/

#include "kx134_1211.h"

#include "string.h"

/* =============================================================================== */
/**
 * @brief Initialiser for a KX134-1211 accelerometer.
 *
 * @param *accel 		Pointer to KX134-1211 struct to be initialised.
 * @param *spi 			Pointer to SPI peripheral struct.
 * @param cs 				Device chip select GPIO.
 * @param scale     Selected scale for read accelerations.
 * @param *axes     Array defining sensor mounting axes.
 * @return @c NULL.
 **
 * =============================================================================== */
KX134_1211_t KX134_1211_init(
  KX134_1211_t *accel,
  SPI_t *spi,
  GPIOpin_t cs,
  uint8_t scale,
  const uint8_t *axes,
  const int8_t *sign
) {
  accel->spi                  = spi;
  accel->cs                   = cs;
  accel->base.dataSize        = KX134_1211_DATA_TOTAL;
  accel->base.update          = KX134_1211_update;
  accel->base.readAccel       = KX134_1211_readAccel;
  accel->base.readRawBytes    = KX134_1211_readRawBytes;
  accel->base.processRawBytes = KX134_1211_processRawBytes;
  accel->base.axes            = accel->axes;
  accel->base.sign            = accel->sign;
  accel->base.accelData       = accel->accelData;
  accel->base.rawAccelData    = accel->rawAccelData;
  memcpy(&accel->axes, axes, KX134_1211_DATA_COUNT);
  memcpy(&accel->sign, sign, KX134_1211_DATA_COUNT);

  // Set value of GSEL and sensitivity based on selected scale
  uint8_t GSEL = 0x00;
  if (scale == 32) {
    GSEL                    = KX134_1211_CNTL1_GSEL(32);
    accel->base.sensitivity = KX134_1211_SENSITIVITY(32);
  } else if (scale == 16) {
    GSEL                    = KX134_1211_CNTL1_GSEL(16);
    accel->base.sensitivity = KX134_1211_SENSITIVITY(16);
  }

  // Perform software reset as per datasheet
  KX134_1211_writeRegister(accel, 0x7F, 0x00);
  KX134_1211_writeRegister(accel, 0x1C, 0x00);
  KX134_1211_writeRegister(accel, 0x1C, 0x80);

  // Wait for the spefified period  - need to wait for 2ms here.
  for (uint32_t i = 0; i < 0xFFFF; i++);

  // Make sure reset was successful - must add real checks here
  // TODO: add in error checking
  uint8_t chipID = KX134_1211_readRegister(accel, 0x13);
  uint8_t cotr   = KX134_1211_readRegister(accel, 0x12);

  // Configure accelerometer registers
  KX134_1211_writeRegister(accel, KX134_1211_CNTL1, KX134_1211_CNTL1_RES | GSEL);                        // Accel select, selected sensitivity
  uint8_t ODCNTL = KX134_1211_readRegister(accel, KX134_1211_ODCNTL);                                    // Read from register for reserve mask
  KX134_1211_writeRegister(accel, KX134_1211_ODCNTL, (KX134_1211_ODCNTL_RESERVED & ODCNTL) | 0x2A);      // No filter, fast startup, 800Hz
  KX134_1211_writeRegister(accel, KX134_1211_CNTL1, KX134_1211_CNTL1_PC1 | KX134_1211_CNTL1_RES | GSEL); // Enable PC1

  return *accel;
}

/********************************** DEVICE METHODS *********************************/

/* =============================================================================== */
/**
 * @brief Read 3-axis floating point accelerations.
 *
 * @param 	*accel 		Pointer to accel struct.
 * @param 	*out 		  Floating point acceleration array.
 * @returns @c NULL.
 **
 * =============================================================================== */
void KX134_1211_readAccel(Accel_t *accel, float *out) {
  accel->update(accel);
  out = accel->accelData;
}

/* =============================================================================== */
/**
 * @brief Update internally stored acceleration readings.
 *
 * @param 	*accel 		Pointer to accel struct.
 * @returns @c NULL.
 **
 * =============================================================================== */
void KX134_1211_update(Accel_t *accel) {
  accel->readRawBytes(accel, accel->rawAccelData);
  accel->processRawBytes(accel, accel->rawAccelData, accel->accelData);
}

/* =============================================================================== */
/**
 * @brief Process raw 3-axis data to floating point accelerations.
 *
 * @param 	*accel 		Pointer to accel struct.
 * @param 	*bytes 		Raw 3-axis data array.
 * @param 	*out 			Processed 3-axis data array to write.
 * @returns @c NULL.
 **
 * =============================================================================== */
void KX134_1211_processRawBytes(Accel_t *accel, uint8_t *bytes, float *out) {
  //
  out[0] = accel->sign[0] * accel->sensitivity * (int16_t)(((uint16_t)bytes[0] << 8) | bytes[1]); // Accel X
  out[1] = accel->sign[1] * accel->sensitivity * (int16_t)(((uint16_t)bytes[2] << 8) | bytes[3]); // Accel Y
  out[2] = accel->sign[2] * accel->sensitivity * (int16_t)(((uint16_t)bytes[4] << 8) | bytes[5]); // Accel Z
}

/* =============================================================================== */
/**
 * @brief Read raw 3-axis data.
 *
 * @param 	*accel 		Pointer to accel struct.
 * @param 	*out 			Raw 3-axis data array to write.
 * @returns @c NULL.
 **
 * =============================================================================== */
void KX134_1211_readRawBytes(Accel_t *accel, uint8_t *out) {
  // Map raw indices to mounting axis
#define INDEX_AXES(index, byte) 2 * accel->axes[index] + byte
  uint8_t tmp[KX134_1211_DATA_TOTAL];
  KX134_1211_readRegisters((KX134_1211_t *)accel, KX134_1211_XOUT_L, KX134_1211_DATA_TOTAL, tmp);
  out[INDEX_AXES(0, 1)] = tmp[0]; // Accel X high
  out[INDEX_AXES(0, 0)] = tmp[1]; // Accel X low
  out[INDEX_AXES(1, 1)] = tmp[2]; // Accel Y high
  out[INDEX_AXES(1, 0)] = tmp[3]; // Accel Y low
  out[INDEX_AXES(2, 1)] = tmp[4]; // Accel Z high
  out[INDEX_AXES(2, 0)] = tmp[5]; // Accel Z low
#undef INDEX_AXES
}

/******************************** INTERFACE METHODS ********************************/

void KX134_1211_writeRegister(KX134_1211_t *accel, uint8_t address, uint8_t data) {
  SPI_t *spi   = accel->spi;
  GPIOpin_t cs = accel->cs;

  // Pull CS low
  cs.reset(&cs);

  // Send read command and address
  uint8_t payload = address & 0x7F; // Load payload with address and write command
  spi->transmit(spi, payload);      // Transmit payload
  spi->transmit(spi, data);         // Transmit write data

  // Set CS high
  cs.set(&cs);
}

uint8_t KX134_1211_readRegister(KX134_1211_t *accel, uint8_t address) {
  uint8_t response = 0;
  SPI_t *spi       = accel->spi;
  GPIOpin_t cs     = accel->cs;

  // Pull CS low
  cs.reset(&cs);

  // Send read command and address
  uint8_t payload = address | 0x80;              // Load payload with address and read command
  response        = spi->transmit(spi, payload); // Transmit payload
  response        = spi->transmit(spi, 0xFF);    // Transmit dummy data and read response data

  // Set CS high
  cs.set(&cs);

  return response;
}

void KX134_1211_readRegisters(KX134_1211_t *accel, uint8_t address, uint8_t count, uint8_t *out) {
  SPI_t *spi   = accel->spi;
  GPIOpin_t cs = accel->cs;

  // Pull CS low
  cs.reset(&cs);

  // Send read command and address
  uint8_t payload = address | 0x80; // Load payload with address and read command
  spi->transmit(spi, payload);      // Transmit payload

  // Auto increment read through registers
  for (uint8_t i = 0; i < count; i++) {
    out[i] = spi->transmit(spi, 0xFF);
  }

  // Set CS high
  cs.set(&cs);
}

/** @} */
