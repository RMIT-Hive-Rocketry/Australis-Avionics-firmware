/**
 * @author Matt Ricci
 * @addtogroup System
 */

// ALLOW FORMATTING
#ifndef _DEVICELIST_H
#define _DEVICELIST_H

/**
 * @ingroup System
 * @addtogroup Device_Management Device Management
 * @brief
 * @{
 */

// TODO:
// Rename struct to Device and refactor handle
// to be a typedef of the Device pointer
typedef struct {
  char *deviceName; // Name of device
  void *device;     // Pointer to device struct
} DeviceHandle_t;

// TODO: Remove non-core device keys
typedef enum {
  DEVICE_ACCEL_HIGH, // High range accelerometer
  DEVICE_ACCEL_LOW,  // Low range accelerometer
  DEVICE_ACCEL,      // Current accelerometer
  DEVICE_GYRO,       // Gyroscope
  DEVICE_BARO,       // Barometer
  DEVICE_FLASH,      // Flash storage
  DEVICE_LORA,       // LoRa transceiver
  DEVICE_GPS,        // GPS module
  DEVICE_UART_USB,   // UART via FTDI
  DEVICE_MAX_KEYS
} DeviceKey;

void DeviceList_init(DeviceHandle_t deviceList[DEVICE_MAX_KEYS]);
DeviceHandle_t DeviceList_getDeviceHandle(DeviceKey);
DeviceHandle_t DeviceList_getDeviceHandleFromName(char *);
DeviceHandle_t *DeviceList_getDeviceHandlePointer(DeviceKey);
void DeviceList_printDevices();

#endif

/** @} */
