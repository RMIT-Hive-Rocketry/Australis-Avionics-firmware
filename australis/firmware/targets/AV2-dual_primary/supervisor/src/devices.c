/***********************************************************************************
 * @file        devices.c                                                          *
 * @author      Matt Ricci                                                         *
 * @brief       Provides device and peripheral driver initialisations.             *
 ***********************************************************************************/

#include "devices.h"

#include "AustralisConfig.h"
#include "devicelist.h"

#include "spi.h"
#include "stddef.h"
#include "string.h"

// Peripheral includes
#include "gpiopin.h"

// Device includes
#include "ais3624dq.h"
#include "iam_20380.h"
#include "lps22df.h"
#include "w25q128.h"

#include "dataframe.h"

static DeviceHandle_t deviceList[DEVICE_MAX_KEYS];
static SPI_t spi;

static void initSpi(void);
static bool initSensors(void);
static bool initFlash(void);

/* ============================================================================================== */
/**
 * @brief Initialise and store device drivers.
 *
 * @return .
 **
 * ============================================================================================== */
bool initDevices(void) {
  DeviceList_init(deviceList);

  // SPI peripherals and devices
  initSpi();
  initSensors();
  initFlash();

  // @TODO: add in error checking
  return true;
}

/* ============================================================================================== */
/**
 * @brief   Initialise SPI bus pins.
 * @details Initialises GPIO pins for SCK, SDI and SDO on each bus. See \ref devices.h for Port/pin,
 *          alternate function selections and pin mapping definitions.
 *
 * @return  \c NULL.
 **
 * ============================================================================================== */
void initSpi(void) {
  // Supervisor SPI configuration
  GPIO_Config spiSupPinConfig = GPIO_CONFIG_DEFAULT;
  spiSupPinConfig.mode        = GPIO_MODE_AF;
  spiSupPinConfig.afr         = SUP_SPI_AF;

  GPIOpin_t supervisorSCK     = GPIOpin_init(SUP_SPI_PORT, SUP_SPI_SCK, &spiSupPinConfig);
  GPIOpin_t supervisorSDI     = GPIOpin_init(SUP_SPI_PORT, SUP_SPI_SDI, &spiSupPinConfig);
  GPIOpin_t supervisorSDO     = GPIOpin_init(SUP_SPI_PORT, SUP_SPI_SDO, &spiSupPinConfig);

  SPI_Config spiSupConfig     = SPI_CONFIG_DEFAULT;
  spiSupConfig.BR             = SPI_BR_PCLK16;
  spi                         = SPI_init(SUP_SPI_INTERFACE, &spiSupConfig);
}

/* ============================================================================================== */
/**
 * @brief Initialise and store device drivers.
 *
 * @return .
 **
 * ============================================================================================== */
bool initSensors() {

  // ==========================================================================
  // HIGH RANGE ACCELEROMETER
  //
  // Provides low resolution (high scale) inertial data for 3-axis acceleration.
  // This is used for data logging as well as state estimation when
  // the rocket's upward velocity is greater than the maximum high
  // resolution scale.
  GPIOpin_t hAccelCS = GPIOpin_init(ACCEL_CS2, NULL);
  static AIS3624DQ_t hAccel;
  AIS3624DQ_init(
    &hAccel,
    &spi,
    hAccelCS,
    ACCEL_SCALE_HIGH, // Set to high scale for larger G forces
    ACCEL_AXES_2,     // Accelerometer 2 mounting axes
    ACCEL_SIGN_2      // +/- for mounting axes
  );
  deviceList[DEVICE_ACCEL_HIGH].deviceName = "HAccel";
  deviceList[DEVICE_ACCEL_HIGH].device     = &hAccel;

  // ==========================================================================
  // LOW RANGE ACCELEROMETER
  //
  // Provides high resolution (low scale) inertial data for 3-axis acceleration.
  // This is used for data logging as well as state estimation when
  // the rocket's upward velocity is within range of the high resolution.
  GPIOpin_t lAccelCS = GPIOpin_init(ACCEL_CS1, NULL);
  static AIS3624DQ_t lAccel;
  AIS3624DQ_init(
    &lAccel,
    &spi,
    lAccelCS,
    ACCEL_SCALE_LOW, // Set to low scale for smaller G forces
    ACCEL_AXES_1,    // Accelerometer 1 mounting axes
    ACCEL_SIGN_1     // +/- for mounting axes
  );
  deviceList[DEVICE_ACCEL_LOW].deviceName = "LAccel";
  deviceList[DEVICE_ACCEL_LOW].device     = &lAccel;

  // ==========================================================================
  // BAROMETER
  //
  // Measures temperature compensated atmospheric pressure. Data from the
  // barometer is used in altitude calculation, as well as providing one
  // metric for detecting apogee (decreasing pressure readings).
  // TODO: Implement secondary baro driver and adjust init
  GPIOpin_t baroCS = GPIOpin_init(BARO_CS, NULL);
  static LPS22DF_t baro;
  LPS22DF_init(
    &baro,
    &spi,
    baroCS,
    LPS22DF_TEMP_SENSITIVITY, // Set temperature measurement sensitivity
    LPS22DF_PRESS_SENSITIVITY // Set pressure measurement sensitivity
  );
  deviceList[DEVICE_BARO].deviceName = "Baro";
  deviceList[DEVICE_BARO].device     = &baro;

  // ==========================================================================
  // GYROSCOPE
  //
  // Measures inertial data for 3-axis rotations. This data is used in
  // calculations for attitude quaternion to determine rotation during flight
  // and apply tilt-angle compensation.
  // TODO: Implement secondary gyro driver and adjust init
  GPIOpin_t gyroCS = GPIOpin_init(GYRO_CS, NULL);
  static IAM_20380_t gyro;
  IAM_20380_init(
    &gyro,
    &spi,
    gyroCS,
    IAM_20380_SENSITIVITY_FS_SEL500, // Set measurement sensitivity
    GYRO_AXES,                       // Gyroscope mounting axes
    GYRO_SIGN                        // +/- for mounting axes
  );
  deviceList[DEVICE_GYRO].deviceName = "Gyro";
  deviceList[DEVICE_GYRO].device     = &gyro;

  // ==========================================================================
  // !! THIS DEVICE HANDLE WILL BE MODIFIED AT RUN-TIME !!
  // DEVICE_ACCEL represents the current accelerometer selected by the system.
  //
  // The intent is that the high frequency data task will determine which of the
  // two connected accelerometers (configured for high and low resolution) will be
  // used for data acquisition. Other tasks should interact with the current
  // accelerometer without directly modifying it.
  deviceList[DEVICE_ACCEL] = deviceList[DEVICE_ACCEL_LOW];

  // @TODO: add in error checking
  return true;
}

/* ============================================================================================== */
/**
 * @brief Initialise and store flash device driver.
 *
 * @return .
 **
 * ============================================================================================== */
bool initFlash() {

  // ==========================================================================
  // FLASH
  //
  // Flash storage for measured and calculated data during flight. Data is
  // written to the device in dataframe chunks. Each interval of high and
  // low resolution data is appended in a frame to a circular memory buffer
  // by the system, and is written to flash during idle time once at least a
  // full page of data is available.
  GPIOpin_t flashCS = GPIOpin_init(FLASH_CS_PORT, FLASH_CS_PIN, NULL);
  static W25Q128_t flash;
  W25Q128_init(
    &flash,
    &spi,
    flashCS
  );
  deviceList[DEVICE_FLASH].deviceName = "Flash";
  deviceList[DEVICE_FLASH].device     = &flash;

  // @TODO: add in error checking
  return true;
}
