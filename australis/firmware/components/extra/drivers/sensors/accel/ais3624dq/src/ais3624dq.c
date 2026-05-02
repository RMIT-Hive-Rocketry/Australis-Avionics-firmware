/***********************************************************************************
 * @file        ais3624dq.c                                                        *
 * @author      Matt Ricci                                                         *
 * @addtogroup  AIS3624DQ                                                          *
 * @{                                                                              *
 ***********************************************************************************/

#include "ais3624dq.h"

#include "string.h"

static void AIS3624DQ_readRegisters(AIS3624DQ_t *, uint8_t, uint8_t, uint8_t *);
static uint8_t AIS3624DQ_readRegister(AIS3624DQ_t *, uint8_t);
static void AIS3624DQ_writeRegister(AIS3624DQ_t *, uint8_t, uint8_t);

/* =============================================================================== */
/**
 * @brief Initialiser for a AIS3624DQ accelerometer.
 *
 * @param *accel 		Pointer to AIS3624DQ struct to be initialised.
 * @param *spi 			Pointer to SPI peripheral struct.
 * @param cs 				Device chip select GPIO.
 * @param scale     Selected scale for read accelerations.
 * @param *axes     Array defining sensor mounting axes.
 * @return @c NULL.
 **
 * =============================================================================== */
// TODO: Replace 'scale' uint8_t argument with device specific enum typedef
AIS3624DQ_t AIS3624DQ_init(
  AIS3624DQ_t *accel,
  SPI_t *spi,
  GPIOpin_t cs,
  uint8_t scale,
  const uint8_t *axes,
  const int8_t *sign
) {
  accel->spi                  = spi;
  accel->cs                   = cs;
  accel->base.dataSize        = AIS3624DQ_DATA_TOTAL;
  accel->base.update          = AIS3624DQ_update;
  accel->base.readAccel       = AIS3624DQ_readAccel;
  accel->base.readRawBytes    = AIS3624DQ_readRawBytes;
  accel->base.processRawBytes = AIS3624DQ_processRawBytes;
  accel->base.axes            = accel->axes;
  accel->base.sign            = accel->sign;
  accel->base.accelData       = accel->accelData;
  accel->base.rawAccelData    = accel->rawAccelData;
  memcpy(accel->base.axes, axes, AIS3624DQ_DATA_COUNT);
  memcpy(accel->base.sign, sign, AIS3624DQ_DATA_COUNT);

  uint8_t FS = 0x00;
  if (scale == 24) {
    // 24G scale
    FS                      = AIS3624DQ_CTRL_REG4_FS(24);
    accel->base.sensitivity = AIS3624DQ_SENSITIVITY(24);
  } else if (scale == 12) {
    // 12G scale
    FS                      = AIS3624DQ_CTRL_REG4_FS(12);
    accel->base.sensitivity = AIS3624DQ_SENSITIVITY(12);
  } else if (scale == 6) {
    // 6G scale
    FS                      = AIS3624DQ_CTRL_REG4_FS(6);
    accel->base.sensitivity = AIS3624DQ_SENSITIVITY(6);
  }

  AIS3624DQ_writeRegister(
    accel,
    AIS3624DQ_CTRL_REG1,
    (AIS3624DQ_CTRL_REG1_DR_1000     // 1kHz datarate
     | AIS3624DQ_CTRL_REG1_PM_NORMAL // Power mode normal
     | AIS3624DQ_CTRL_REG1_XEN       // Enable x-axis measurement
     | AIS3624DQ_CTRL_REG1_YEN       // Enable y-axis measurement
     | AIS3624DQ_CTRL_REG1_ZEN)      // Enable z-axis measurement
  );

  // Set scale, enable block data updates
  AIS3624DQ_writeRegister(accel, AIS3624DQ_CTRL_REG4, FS | AIS3624DQ_CTRL_REG4_BDU);

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
void AIS3624DQ_readAccel(Accel_t *accel, float *out) {
  accel->update(accel);
  out = accel->accelData;
}

/* =============================================================================== */
/**
 * @brief Updates internally stored acceleration readings.
 *
 * @param 	*accel 		Pointer to accel struct.
 * @returns @c NULL.
 **
 * =============================================================================== */
void AIS3624DQ_update(Accel_t *accel) {
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
void AIS3624DQ_processRawBytes(Accel_t *accel, uint8_t *bytes, float *out) {
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
void AIS3624DQ_readRawBytes(Accel_t *accel, uint8_t *out) {
  // Map raw indices to mounting axis
#define INDEX_AXES(index, byte) 2 * accel->axes[index] + byte
  uint8_t tmp[AIS3624DQ_DATA_TOTAL];
  AIS3624DQ_readRegisters((AIS3624DQ_t *)accel, AIS3624DQ_XOUT_L, AIS3624DQ_DATA_TOTAL, tmp);
  out[INDEX_AXES(0, 1)] = tmp[0]; // Accel X high
  out[INDEX_AXES(0, 0)] = tmp[1]; // Accel X low
  out[INDEX_AXES(1, 1)] = tmp[2]; // Accel Y high
  out[INDEX_AXES(1, 0)] = tmp[3]; // Accel Y low
  out[INDEX_AXES(2, 1)] = tmp[4]; // Accel Z high
  out[INDEX_AXES(2, 0)] = tmp[5]; // Accel Z low
#undef INDEX_AXES
}

/******************************** INTERFACE METHODS ********************************/

void AIS3624DQ_writeRegister(AIS3624DQ_t *accel, uint8_t address, uint8_t data) {
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

uint8_t AIS3624DQ_readRegister(AIS3624DQ_t *accel, uint8_t address) {
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

void AIS3624DQ_readRegisters(AIS3624DQ_t *accel, uint8_t address, uint8_t count, uint8_t *out) {
  SPI_t *spi   = accel->spi;
  GPIOpin_t cs = accel->cs;

  // Pull CS low
  cs.reset(&cs);

  // Send read command and address
  uint8_t payload = address | 0xC0; // Load payload with address, MS and read command
  spi->transmit(spi, payload);      // Transmit payload

  // Auto increment read through registers
  for (uint8_t i = 0; i < count; i++) {
    out[i] = spi->transmit(spi, 0xFF);
  }

  // Set CS high
  cs.set(&cs);
}

/** @} */
