/**
 * @file accelerometer.h
 * @brief Defines the API for the Accelerometer sensor.
 * @addtogroup API_accel
 * @{
 */

// ALLOW FORMATTING
#ifndef ACCEL_H
#define ACCEL_H

#include "stdint.h"

typedef struct Accel {
  /**
   ******************************************************
   * @brief Pointer to update method.
   *
   * Update internally stored acceleration readings.
   *
   * @param 	*accel 		Pointer to accel struct.
   * @returns @c NULL.
   ******************************************************/
  void (*update)(struct Accel *accel);

  /**
   ******************************************************
   * @brief Pointer to readAccel method.
   *
   * Read 3-axis floating point accelerations.
   *
   * @param 	*accel 		Pointer to accel struct.
   * @param 	*out 		  Floating point acceleration array.
   * @returns @c NULL.
   ******************************************************/
  void (*readAccel)(struct Accel *accel, float *out);

  /**
   ******************************************************
   * @brief Pointer to readRawBytes method.
   *
   * Read raw 3-axis data.
   *
   * @param 	*accel 		Pointer to accel struct.
   * @param 	*out 			Raw 3-axis data array to write.
   * @returns @c NULL.
   ******************************************************/
  void (*readRawBytes)(struct Accel *accel, uint8_t *out);

  /**
   ******************************************************
   * @brief Pointer to processRawBytes method.
   *
   * Process raw 3-axis data to floating point accelerations.
   *
   * @param 	*accel 		Pointer to accel struct.
   * @param 	*bytes 		Raw 3-axis data array.
   * @param 	*out 			Processed 3-axis data array to write.
   * @returns @c NULL.
   ******************************************************/
  void (*processRawBytes)(struct Accel *accel, uint8_t *bytes, float *out);

  uint8_t dataSize;      //!< Total data size.
  uint8_t *axes;         //!< Pointer to driver defined axes
  int8_t *sign;          //!< Pointer to driver defined signs
  uint8_t *rawAccelData; //!< Pointer to driver defined raw data array
  float *accelData;      //!< Pointer to driver defined data array
  float sensitivity;
} Accel_t;

#endif
/** @} */
