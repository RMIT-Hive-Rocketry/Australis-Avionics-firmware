/***********************************************************************************
 * @file        state.c                                                            *
 * @author      Matt Ricci                                                         *
 * @addtogroup  State                                                              *
 *                                                                                 *
 * @{                                                                              *
 ***********************************************************************************/

#include "state.h"

#define AVG_BUFF_SIZE   15
#define MEM_BUFF_SIZE   20992
#define FLASH_PAGE_SIZE 256

float avgPressBuff[AVG_BUFF_SIZE];
float avgVelBuff[AVG_BUFF_SIZE];
uint8_t buff[MEM_BUFF_SIZE];

// TODO:
// Move state pointer getter to private interface only accessible to core
// and implement public interface that returns copy of struct.
//
// Australis Core should be the only source codebase that has access to
// modify the contents of the system state variables. Updating state is
// not an atomic operation, however with only a single writing task
// per variable, and no possiblity for re-entrance, this will not pose
// an issue to thread safety.
//
// This behaviour should be documented with an extremely clear warning
// for internal development to avoid adding more tasks that may write
// to state variables with already assigned ownership.

static State state = {};

/* =============================================================================== */
/**
 * @brief
 *
 * @param
 *
 * @return
 **
 * =============================================================================== */
void State_init() {
  state.flightState  = PRELAUNCH;
  state.flightTimeMs = 0;
  state.tilt         = 0.0f;
  state.cosine       = 0.0f;
  state.altitude     = 0.0f;
  state.velocity     = 0.0f;
  state.rotation     = Quaternion_new();
  state.avgVel       = SlidingWindow_new(avgVelBuff, AVG_BUFF_SIZE);
  state.avgPress     = SlidingWindow_new(avgVelBuff, AVG_BUFF_SIZE);
  state.mem          = MemBuff_new(buff, MEM_BUFF_SIZE, FLASH_PAGE_SIZE);
  memcpy(state.attitude, (float[]){0, 0, 1}, sizeof(state.attitude));
  memcpy(state.launchAngle, (float[]){0, 0, 1}, sizeof(state.launchAngle));
}

/* =============================================================================== */
/**
 * @brief
 *
 * @param
 *
 * @return
 **
 * =============================================================================== */
State *State_getState() {
  return &state;
}
