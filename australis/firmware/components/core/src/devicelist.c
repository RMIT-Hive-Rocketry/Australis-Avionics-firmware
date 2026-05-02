/***********************************************************************************
 * @file        devicelist.c                                                       *
 * @author      Matt Ricci                                                         *
 * @addtogroup  Device_Management                                                  *
 *                                                                                 *
 * @{                                                                              *
 ***********************************************************************************/

#include "devicelist.h"

#include "stddef.h"
#include "string.h"
#include <AustralisCore.h>

// TODO:
// Add support for extended device access via static linked list
// for non-core devices. These would only be accessible by name,
// and mostly useful for runtime access (e.g. in a shell program)

DeviceHandle_t deviceList[DEVICE_MAX_KEYS];
static DeviceHandle_t *deviceListPtr = deviceList;

/* =============================================================================== */
/**
 * @brief Initialise all system devices.
 *
 * @param  deviceList Empty list to be initialised.
 *
 * TODO:
 * Refactor and remove option for user defined list.
 * This will require adjustments to existing target code.
 **
 * =============================================================================== */
void DeviceList_init(DeviceHandle_t deviceList[DEVICE_MAX_KEYS]) {
  deviceListPtr = deviceList;
}

/* =============================================================================== */
/**
 * @brief Retrieve device handle from list by key.
 *
 * @param  key Key index for device in list.
 *
 * @return Device handle if valid key is provided, else invalid handle.
 **
 * =============================================================================== */
DeviceHandle_t DeviceList_getDeviceHandle(DeviceKey key) {

  ASSERT(key < DEVICE_MAX_KEYS);
  ASSERT(deviceListPtr[key].device);

  return deviceListPtr[key];
}

/* =============================================================================== */
/**
 * @brief Retrieve device handle from list by name.
 *
 * @param  name String representation of device to be retrieved.
 *
 * @return Device handle if device with matching name is found, else invalid handle.
 **
 * =============================================================================== */

DeviceHandle_t DeviceList_getDeviceHandleFromName(char *name) {
  // Iterate list searching for matching device
  for (int i = 0; i < DEVICE_MAX_KEYS; i++) {
    if (!strcmp(deviceListPtr[i].deviceName, name)) {
      // Return handle if found
      return deviceListPtr[i];
    }
  }

  // Otherwise, return empty handle
  return (DeviceHandle_t){"NOTFOUND", NULL};
}

/* =============================================================================== */
/**
 * @brief Retrieve device handle pointer from list by key.
 *
 * @important Caution should be taken when making use of this method as modifying
 * device handle pointers during run-time will lead to side effects.
 *
 * @note Device handles that are intended to be modified should be clearly documented
 * at the point of initialisation to indicate that the behaviour of code accessing
 * this handle will be non-deterministic.
 * @note Justification for run-time manipulation of device handles should likewise
 * be documented.
 *
 * @param  key Key index for device in list.
 *
 * @return Pointer to device handle if valid key is provided, else \c NULL.
 *
 **
 * =============================================================================== */
DeviceHandle_t *DeviceList_getDeviceHandlePointer(DeviceKey key) {

  ASSERT(key < DEVICE_MAX_KEYS);
  ASSERT(deviceListPtr[key].device);

  return &deviceListPtr[key];
}

/* =============================================================================== */
/**
 * @brief Print out names of all devices in list.
 *
 **
 * =============================================================================== */
void DeviceList_printDevices() {
  /* TODO: Must implement printf retarget
  for (int i = 0; i < DEVICE_MAX_KEYS; i++)
    printf("%s\n", deviceListPtr[i].deviceName);
  */
}

/** @} */
