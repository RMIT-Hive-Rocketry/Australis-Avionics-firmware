/**
 * @addtogroup System
 */

// ALLOW FORMATTING
#ifndef STATE_H
#define STATE_H

#include "membuff.h"
#include "quaternion.h"
#include "slidingwindow.h"

/**
 * @ingroup System
 * @addtogroup State
 * @brief
 * @{
 */

/**
 * @brief   Flight state enum
 * @details Describes the current phase of flight
 */
enum State {
  PRELAUNCH, //!< Initial boot condition
  LAUNCH,    //!< Body reference Z-axis acceleration above threshold
  COAST,     //!< Global reference Z-axis velocity decreasing
  APOGEE,    //!< At least 2: velocity negative, pressure increasing, tilt > 90 degrees
  DESCENT    //!< Altitude below main threshold
};

/**
 * @brief   State variable struct
 * @details Maintains current values for state variables
 */
typedef struct {
  enum State flightState;
  uint32_t flightTimeMs;
  float attitude[3];
  float launchAngle[3];
  float tilt;
  float cosine;
  float altitude;
  float velocity;
  Quaternion rotation;
  SlidingWindow avgVel;
  SlidingWindow avgPress;
  MemBuff mem;
} State;

void State_init();
State *State_getState();

#endif
