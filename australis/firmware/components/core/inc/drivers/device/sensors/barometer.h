/**
 * @file barometer.h
 * @brief Defines the API for the Barometer sensor.
 * @addtogroup API_baro
 * @{
 */

// ALLOW FORMATTING
#ifndef BARO_H
#define BARO_H

#include "stdint.h"

typedef struct Baro {
  /**
   ******************************************************
   * @brief Pointer to update method.
   *
   * Update internally stored pressure and temperature
   * readings.
   *
   * @param 	*baro 		Pointer to baro struct.
   * @returns @c NULL.
   ******************************************************/
  void (*update)(struct Baro *baro);

  /**
   ******************************************************
   * @brief Pointer to readTemp method.
   *
   * Read processed floating point temperature.
   *
   * @param 	*baro 		Pointer to baro struct.
   * @param 	*out 		  Floating point temperature value
   *                    pointer to write.
   * @returns @c NULL.
   ******************************************************/
  void (*readTemp)(struct Baro *baro, float *out);

  /**
   ******************************************************
   * @brief Pointer to readPress method.
   *
   * Read processed floating point pressure.
   *
   * @param 	*baro 		Pointer to baro struct.
   * @param 	*out 		  Floating point pressure value
   *                    pointer to write.
   * @returns @c NULL.
   ******************************************************/
  void (*readPress)(struct Baro *baro, float *out);

  /**
   ******************************************************
   * @brief Pointer to readRawTemp method.
   *
   * Read raw temperature data bytes.
   *
   * @param 	*baro 		Pointer to baro struct.
   * @param 	*out 			Raw temperature data byte array
   *                    to write.
   * @returns @c NULL.
   ******************************************************/
  void (*readRawTemp)(struct Baro *baro, uint8_t *out);

  /**
   ******************************************************
   * @brief Pointer to readRawPress method.
   *
   * Read raw pressure data bytes.
   *
   * @param 	*baro 		Pointer to baro struct.
   * @param 	*out 			Raw pressure data byte array to
   *                    write.
   * @returns @c NULL.
   ******************************************************/
  void (*readRawPress)(struct Baro *baro, uint8_t *out);

  /**
   ******************************************************
   * @brief Pointer to processRawTemp method.
   *
   * Process raw temperature data bytes to floating
   * point temperature.
   *
   * @param 	*baro 		Pointer to baro struct.
   * @param 	*bytes 		Raw temperature data byte array.
   * @param 	*out 			Processed floating point
   *                    temperature value pointer to write.
   * @returns @c NULL.
   ******************************************************/
  void (*processRawTemp)(struct Baro *baro, uint8_t *bytes, float *out);

  /**
   ******************************************************
   * @brief Pointer to processRawPress method.
   *
   * Process raw pressure data bytes to floating point
   * pressure.
   *
   * @param 	*baro 		Pointer to baro struct.
   * @param 	*bytes 		Raw pressure data byte array.
   * @param 	*out 			Processed floating point pressure
   *                    value pointer to write.
   * @returns @c NULL.
   ******************************************************/
  void (*processRawPress)(struct Baro *baro, uint8_t *bytes, float *out);

  uint8_t pressDataSize; //!< Size of raw pressure data in bytes.
  uint8_t tempDataSize;  //!< Size of raw temperature data in bytes.
  uint8_t *rawPress;     //!< Pointer to driver defined raw pressure data array.
  uint8_t *rawTemp;      //!< Pointer to driver defined raw temperature data array.
  float groundPress;     //!< Stored ground pressure reading
  float press;           //!< Last read processed pressure value.
  float temp;            //!< Last read processed temperature value.
  float sensitivity;
} Baro_t;

/** @} */
#endif
