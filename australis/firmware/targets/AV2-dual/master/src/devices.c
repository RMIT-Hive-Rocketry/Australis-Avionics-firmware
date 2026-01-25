/***********************************************************************************
 * @file        devices.c                                                          *
 * @author      Matt Ricci                                                         *
 * @brief       Provides device and peripheral driver initialisations.             *
 ***********************************************************************************/

#include "devices.h"

// Core includes
#include "AustralisConfig.h"
#include "devicelist.h"

// Peripheral includes
#include "gpiopin.h"
#include "spi.h"
#include "uart.h"

// Device includes
#include "a3g4250d.h"
#include "bmp581.h"
#include "kx134_1211.h"
#include "sam_m10q.h"
#include "sx1272.h"
#include "w25q128.h"

#include "lorapub.h"

static DeviceHandle_t deviceList[DEVICE_MAX_KEYS];

static bool initSensors();
static bool initFlash();
static bool initLora();
static bool initUart();

/* ============================================================================================== */
/**
 * @brief Initialise and store device drivers.
 *
 * @return .
 **
 * ============================================================================================== */
bool initDevices() {
  DeviceList_init(deviceList);

  // SPI peripherals and devices
  initSensors();
  initFlash();
  initLora();

  // UART peripherals and devices
  initUart();

  // @TODO: add in error checking
  return true;
}

/* ============================================================================================== */
/**
 * @brief Initialise and store device drivers.
 *
 * @return .
 **
 * ============================================================================================== */
bool initSensors() {

  // SPI pin configuration
  GPIO_Config spiPinConfig = GPIO_CONFIG_DEFAULT;
  spiPinConfig.mode        = GPIO_MODE_AF;
  spiPinConfig.afr         = SENSORS_SPI_AF;

  // Initialise SCK/SDI/SDO pins
  GPIOpin_t sensorSCK = GPIOpin_init(SENSORS_SPI_PORT, SENSORS_SPI_SCK, &spiPinConfig);
  GPIOpin_t sensorSDI = GPIOpin_init(SENSORS_SPI_PORT, SENSORS_SPI_SDI, &spiPinConfig);
  GPIOpin_t sensorSDO = GPIOpin_init(SENSORS_SPI_PORT, SENSORS_SPI_SDO, &spiPinConfig);

  // Initialise SPI interface
  static SPI_t spiSensors;
  spiSensors = SPI_init(SENSORS_SPI_INTERFACE, NULL);

  // ==========================================================================
  // HIGH RANGE ACCELEROMETER
  //
  // Provides low resolution (high scale) inertial data for 3-axis acceleration.
  // This is used for data logging as well as state estimation when
  // the rocket's upward velocity is greater than the maximum high
  // resolution scale.

  GPIOpin_t hAccelCS = GPIOpin_init(ACCEL_CS2, NULL);

  static KX134_1211_t hAccel;
  KX134_1211_init(
    &hAccel,
    &spiSensors,
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

  static KX134_1211_t lAccel;
  KX134_1211_init(
    &lAccel,
    &spiSensors,
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

  GPIOpin_t baroCS = GPIOpin_init(BARO_CS, NULL);
  static BMP581_t baro;
  BMP581_init(
    &baro,
    &spiSensors,
    baroCS,
    BMP581_TEMP_SENSITIVITY, // Set temperature measurement sensitivity
    BMP581_PRESS_SENSITIVITY // Set pressure measurement sensitivity
  );
  deviceList[DEVICE_BARO].deviceName = "Baro";
  deviceList[DEVICE_BARO].device     = &baro;

  // ==========================================================================
  // GYROSCOPE
  //
  // Measures inertial data for 3-axis rotations. This data is used in
  // calculations for attitude quaternion to determine rotation during flight
  // and apply tilt-angle compensation.

  GPIOpin_t gyroCS = GPIOpin_init(GYRO_CS, NULL);

  static A3G4250D_t gyro;
  A3G4250D_init(
    &gyro,
    &spiSensors,
    gyroCS,
    A3G4250D_SENSITIVITY, // Set measurement sensitivity
    GYRO_AXES,            // Gyroscope mounting axes
    GYRO_SIGN             // +/- for mounting axes
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

  // SPI pin configuration
  GPIO_Config spiPinConfig = GPIO_CONFIG_DEFAULT;
  spiPinConfig.mode        = GPIO_MODE_AF;
  spiPinConfig.afr         = FLASH_SPI_AF;

  // Initialise SCK/SDI/SDO pins
  GPIOpin_t flashSCK = GPIOpin_init(FLASH_SPI_PORT, FLASH_SPI_SCK, &spiPinConfig);
  GPIOpin_t flashSDI = GPIOpin_init(FLASH_SPI_PORT, FLASH_SPI_SDI, &spiPinConfig);
  GPIOpin_t flashSDO = GPIOpin_init(FLASH_SPI_PORT, FLASH_SPI_SDO, &spiPinConfig);

  // Initialise SPI interface
  static SPI_t spiFlash;
  spiFlash = SPI_init(FLASH_SPI_INTERFACE, NULL);

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
    &spiFlash,
    flashCS
  );
  deviceList[DEVICE_FLASH].deviceName = "Flash";
  deviceList[DEVICE_FLASH].device     = &flash;

  // @TODO: add in error checking
  return true;
}

/* ============================================================================================== */
/**
 * @brief Initialise and store LoRa device driver.
 *
 * @return .
 **
 * ============================================================================================== */
bool initLora() {

  // SPI pin configuration
  GPIO_Config spiPinConfig = GPIO_CONFIG_DEFAULT;
  spiPinConfig.mode        = GPIO_MODE_AF;
  spiPinConfig.afr         = LORA_SPI_AF;

  // Initialise SCK/SDI/SDO pins
  GPIOpin_t loraSCK = GPIOpin_init(LORA_SPI_PORT, LORA_SPI_SCK, &spiPinConfig);
  GPIOpin_t loraSDI = GPIOpin_init(LORA_SPI_PORT, LORA_SPI_SDI, &spiPinConfig);
  GPIOpin_t loraSDO = GPIOpin_init(LORA_SPI_PORT, LORA_SPI_SDO, &spiPinConfig);

  // Initialise GPIO to SX1272 DIO as input
  GPIOpin_t loraDIO = GPIOpin_init(GPIOD, GPIO_PIN1, &GPIO_CONFIG_INPUT);

  // Initialise SPI interface
  static SPI_t spiLora;
  SPI_Config spiLoraConfig = SPI_CONFIG_DEFAULT; // Using default settings as base
  spiLoraConfig.CPHA       = SPI_CPHA_FIRST;     // Begin on first clock edge
  spiLoraConfig.CPOL       = SPI_CPOL0;          // Idle clock low
  spiLora                  = SPI_init(LORA_SPI_INTERFACE, &spiLoraConfig);

  // ==========================================================================
  // LORA
  //
  // LoRa transceiver for external wireless communicatons. Can be configured to
  // either receive or transmit data.

  GPIOpin_t loraCS = GPIOpin_init(LORA_CS, NULL);
  static GPIOpin_t rfToggle;

  static SX1272_t lora;
  SX1272_init(
    &lora,
    &spiLora,
    loraCS,
    NULL
  );
  deviceList[DEVICE_LORA].deviceName = "LoRa";
  deviceList[DEVICE_LORA].device     = &lora;
  lora.base.startReceive((LoRa_t *)&lora);

  // Assign transceiver for publication
  LoRa_setTransceiver((LoRa_t *)&lora);

  // Assign RF front-end toggle for publication
  rfToggle = GPIOpin_init(LORA_RF_TOGGLE_PORT, LORA_RF_TOGGLE_PIN, NULL);
  LoRa_setRfToggle(&rfToggle);

  // @TODO: add in error checking
  return true;
}

/* ============================================================================================== */
/**
 * @brief Initialise and store UART device drivers.
 *
 * @return .
 **
 * ============================================================================================== */
bool initUart() {

  UART_Config uartConfig      = UART_CONFIG_DEFAULT;
  uartConfig.RXNEIE           = true; // Enable RXNE interrupt

  GPIO_Config uartTxPinConfig = GPIO_CONFIG_DEFAULT;
  uartTxPinConfig.mode        = GPIO_MODE_AF;
  uartTxPinConfig.afr         = GPIO_AF7;

  GPIO_Config uartRxPinConfig = uartTxPinConfig;
  uartRxPinConfig.pupd        = GPIO_PUPD_PULLUP;

  // ==========================================================================
  // USB UART
  //
  // UART device allowing communication via USB through an FTDI bridge. This
  // particular UART output provides interaction to the system via shell and
  // debug print output.

  GPIOpin_init(USB_PORT, USB_TX_PIN, &uartTxPinConfig);
  GPIOpin_init(USB_PORT, USB_RX_PIN, &uartRxPinConfig);

  static UART_t usbUart;
  usbUart = UART_init(
    USB_INTERFACE,
    USB_BAUD,
    &uartConfig
  );
  deviceList[DEVICE_UART_USB].deviceName = "USB";
  deviceList[DEVICE_UART_USB].device     = &usbUart;

  // ==========================================================================
  // GPS
  //
  // GPS device for low frequency positional readings. Commands are sent and
  // data received via the UART interface.

  GPIOpin_init(GPS_PORT, GPS_TX_PIN, &uartTxPinConfig);
  GPIOpin_init(GPS_PORT, GPS_RX_PIN, &uartRxPinConfig);

  static UART_t gpsUart;
  gpsUart = UART_init(
    GPS_INTERFACE,
    9600,
    &uartConfig
  );

  // Initialise GPS reset pin and device driver
  GPIOpin_t gpsRST = GPIOpin_init(GPS_RESET, NULL);
  gpsRST.set(&gpsRST); // Start reset pin high

  static SAM_M10Q_t gps;
  SAM_M10Q_init(&gps, &gpsUart, GPS_BAUD);
  deviceList[DEVICE_GPS].deviceName = "GPS";
  deviceList[DEVICE_GPS].device     = &gps;

  // @TODO: add in error checking
  return true;
}
