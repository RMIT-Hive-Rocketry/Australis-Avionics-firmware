// ALLOW FORMATTING
#ifndef STATEUPDATE_H
#define STATEUPDATE_H

#include "FreeRTOS.h"
#include "event_groups.h"

#define FLIGHT_STATE_BIT_PRELAUNCH 1
#define FLIGHT_STATE_BIT_LAUNCH    2
#define FLIGHT_STATE_BIT_COAST     4
#define FLIGHT_STATE_BIT_APOGEE    8
#define FLIGHT_STATE_BIT_DESCENT   16
extern EventGroupHandle_t xFlightStateGroup;

void vStateUpdate(void *pvParameters);

#endif
