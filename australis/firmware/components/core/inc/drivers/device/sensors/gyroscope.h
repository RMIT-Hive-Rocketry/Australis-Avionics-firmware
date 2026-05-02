/**
 * @file gyroscope.h
 * @brief Defines the API for the Gyroscope sensor.
 * @addtogroup API_gyro
 * @{
 */

// ALLOW FORMATTING
#ifndef GYRO_H
#define GYRO_H

#include "stdint.h"

typedef struct Gyro {
  /**
   ******************************************************
   * @brief Pointer to update method.
   *
   * Update internally stored rotation readings.
   *
   * @param 	*gyro 		Pointer to gyro struct.
   * @returns @c NULL.
   ******************************************************/
  void (*update)(struct Gyro *gyro);

  /**
   ******************************************************
   * @brief Pointer to readGyro method.
   *
   * Read 3-axis floating point rotations.
   *
   * @param 	*gyro 		Pointer to accel struct.
   * @param 	*out 		  Floating point rotation array.
   * @returns @c NULL.
   ******************************************************/
  void (*readGyro)(struct Gyro *gyro, float *out);

  /**
   ******************************************************
   * @brief Pointer to readRawBytes method.
   *
   * Read raw 3-axis data.
   *
   * @param 	*gyro 		Pointer to gyro struct.
   * @param 	*out 			Raw 3-axis data array to write.
   * @returns @c NULL.
   ******************************************************/
  void (*readRawBytes)(struct Gyro *, uint8_t *);

  /**
   ******************************************************
   * @brief Pointer to processRawBytes method.
   *
   * Process raw 3-axis data to floating point rotations.
   *
   * @param 	*gyro 		Pointer to gyro struct.
   * @param 	*bytes 		Raw 3-axis data array.
   * @param 	*out 			Processed 3-axis data array to write.
   * @returns @c NULL.
   ******************************************************/
  void (*processRawBytes)(struct Gyro *, uint8_t *, float *);

  uint8_t dataSize;     //!< Total data size.
  uint8_t *axes;        //!< Array defining axes of mounting
  int8_t *sign;         //!< Array defining sign of axes
  uint8_t *rawGyroData; //!< Raw angular rates array
  float *gyroData;      //!< Processed angular rates array
  float *bias;          //!< Bias offset array
  float sensitivity;
} Gyro_t;

/** @} */
#endif
