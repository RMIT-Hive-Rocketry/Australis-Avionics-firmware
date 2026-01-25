/***********************************************************************************
 * @file  groundcomms.c                                                            *
 * @brief                                                                          *
 *                                                                                 *
 * @{                                                                              *
 ***********************************************************************************/
// TODO: Refactor this entire source file lmao

#include "groundcomms.h"

#include "stdbool.h"

#include "FreeRTOS.h"
#include "queue.h"

#include "topic.h"
#include "packet.h"
#include "state.h"
#include "devicelist.h"
#include "accelerometer.h"
#include "barometer.h"
#include "gyroscope.h"

// TODO: Implement this better
//
// i.e. GPS doesn't need a mf service
// just make target specific sources choose
// how they want to update the device like
// any other device.
#include "sam_m10q.h"

#include "lorapub.h"

static void sendGroundPacket1(uint8_t broadcastBegin);
static void sendGroundPacket2(SAM_M10Q_Data *data);

/* =============================================================================== */
/**
 * @brief AV to ground communications state machine.
 *
 **
 * =============================================================================== */
void vGroundCommStateMachine(void *argument) {
  (void)argument;

  const TickType_t xFrequency = pdMS_TO_TICKS(250);

  // Create subscription to LoRa topic
  static SUBSCRIBE_TOPIC(lora, loraSubInbox, 10, LORA_MSG_LENGTH);
  // Binary array to store LoRa topic articles
  uint8_t loraRxData[LORA_MSG_LENGTH];

  State *state          = State_getState();
  uint8_t broadcastFlag = 0;

  SAM_M10Q_t *gps       = DeviceList_getDeviceHandle(DEVICE_GPS).device;

  for (;;) {

    TickType_t xLastWakeTime = xTaskGetTickCount();

    // Determine action based on the current flight state
    switch (state->flightState) {

    // --- Ground Pre-Launch State ---
    /*
     * PRELAUNCH State: Wait for a specific request from the GCS.
     *
     * In this state, the system blocks indefinitely until it receives a
     * message on the LoRa queue with the specific GCS Request ID.
     */
    case PRELAUNCH:
      // --- Wait For Request ---
      // Block the state machine until a valid request is received from the ground.
      // Unwanted packets are filtered out and ignored.
      if (xQueueReceive(loraSubInbox, loraRxData, xFrequency)) {
        // Check if the received message has the expected GCS Request ID
        switch (loraRxData[LORA_MESSAGE_INDEX_ID]) {

        case LORA_MESSAGE_ID_GCS_REQUEST:
          // --- Transmit AV Data ---
          vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(380));
          sendGroundPacket1(broadcastFlag);
          // --- Transmit GPS Data ---
          vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(380));
          sendGroundPacket2(&gps->sampleData);
          break;
        default:
          // Do not process if packet is invalid
          break;
        }
      }

      break;

    // --- Active Flight States ---
    /*
     * Active Flight States (LAUNCH, COAST, APOGEE, DESCENT):
     *
     * Handles communication during active flight phases.
     * 1. If broadcast hasn't started: Waits for a specific command from GCS
     *    to begin broadcasting telemetry.
     * 2. If broadcast has started: Continuously sends telemetry data to GCS.
     */
    case LAUNCH:
    case COAST:
    case APOGEE:
    case DESCENT:
      if (broadcastFlag) {
        // --- Broadcast Telemetry ---
        // Broadcast has already been started, continuously send data
        sendGroundPacket1(broadcastFlag);
        sendGroundPacket2(&gps->sampleData);
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(150));
      }

      else {
        // --- Wait for the "Start Broadcast" command from GCS ---
        // Block forever waiting for a message on the LoRa receive queue
        if (xQueueReceive(loraSubInbox, loraRxData, xFrequency)) {
          // Check if it's the GCS command (ID match) AND the broadcast flag is set
          if (loraRxData[LORA_MESSAGE_INDEX_ID] == LORA_MESSAGE_ID_GCS_REQUEST) { // Check if broadcast flag byte is non-zero
            // Update broadcast flag
            broadcastFlag = LORA_MESSAGE_BCAST_FLAG;
          }
        }
      }
      break;
    }
  }
}

/* =============================================================================== */
/**
 * @brief Send ground data packet 1
 *
 **
 * =============================================================================== */
void sendGroundPacket1(uint8_t broadcastBegin) {

  Accel_t *lAccel = DeviceList_getDeviceHandle(DEVICE_ACCEL_LOW).device;
  Accel_t *hAccel = DeviceList_getDeviceHandle(DEVICE_ACCEL_HIGH).device;
  Gyro_t *gyro    = DeviceList_getDeviceHandle(DEVICE_GYRO).device;
  State *state    = State_getState();

  // TODO:
  // Remove hard-coded numbers in favour of defined packet
  // and field lengths in header.

  const size_t packetLength = 9;
  const size_t packetSize   = 32;
  uint8_t bytes[packetSize + 1];
  bytes[0]           = packetSize;

  uint8_t stateFlags = (state->flightState << 5);

  {
    // --- Construct Packet ---
    /*
     * The packet is initialised as a Packet struct containing
     * all fields necessary for id 0x03.
     *
     * Scope is limited for this struct to reduce the memory footprint,
     * as data is essentially doubled for the duration of the packet
     * construction process.
     * The packet structure itself contains information on the fields,
     * the length of each, as well as the data contained. This data
     * must be copied to a raw byte array (the bytes variable) for
     * transmission.
     */
    Packet packet =
      {
        .id     = 0x03,
        .length = packetLength,
        .fields = (Field[]){
          {// [0] Flight State
           .size = 1,
           .data = (uint8_t *)&(stateFlags)
          },
          {// [7:2] Low-rate raw accelerometer data
           .size = lAccel->dataSize,
           .data = lAccel->rawAccelData
          },
          {// [13:8] High-rate raw accelerometer data
           .size = hAccel->dataSize,
           .data = hAccel->rawAccelData
          },
          {// [19:14] Raw gyroscope data
           .size = gyro->dataSize,
           .data = gyro->rawGyroData
          },
          {// [23:20] Altitude (big endian)
           .size = 4,
           .data = (uint8_t[]){
             ((uint8_t *)&state->altitude)[3],
             ((uint8_t *)&state->altitude)[2],
             ((uint8_t *)&state->altitude)[1],
             ((uint8_t *)&state->altitude)[0],
           }
          },
          {// [27:24] Velocity (big endian)
           .size = 4,
           .data = (uint8_t[]){
             ((uint8_t *)&state->velocity)[3],
             ((uint8_t *)&state->velocity)[2],
             ((uint8_t *)&state->velocity)[1],
             ((uint8_t *)&state->velocity)[0],
           }
          },
          // TODO: [28] Apo continuity
          {.size = 1, .data = (uint8_t[]){}},
          // TODO: [29] Main continuity
          {.size = 1, .data = (uint8_t[]){}},
          {// [30] Broadcast flag
           .size = 1,
           .data = (uint8_t[]){broadcastBegin}
          },
        }
      };

    // Construct byte array from packet structure
    Packet_asBytes(&packet, &bytes[1], 32);
  }

  // Send packet comment to LoRa author
  Topic_comment(loraTopic, bytes);
}

/* =============================================================================== */
/**
 * @brief Send ground data packet 2
 *
 **
 * =============================================================================== */
void sendGroundPacket2(SAM_M10Q_Data *data) {

  CREATE_MESSAGE(message, 28);

  State *state       = State_getState();
  Quaternion q       = state->rotation;

  uint8_t stateFlags = (state->flightState << 5);

  {
    // --- Construct Packet ---
    /*
     * The packet is initialised as a Packet struct containing
     * all fields necessary for id 0x03.
     *
     * Scope is limited for this struct to reduce the memory footprint,
     * as data is essentially doubled for the duration of the packet
     * construction process.
     * The packet structure itself contains information on the fields,
     * the length of each, as well as the data contained. This data
     * must be copied to a raw byte array (the bytes variable) for
     * transmission.
     *
     * TODO:
     * Is this even worth the overhead? not sure this could count
     * as being much more readable than the alternative...
     * Realistically this is good for implementation with dynamic
     * memory allocation, not so much for an embedded system.
     */
    Packet packet =
      {
        .id     = 0x04,
        .length = 8,
        .fields = (Field[]){
          {// [0] Flight State
           .size = 1,
           .data = (uint8_t *)&(stateFlags)
          },
          {// [3:0] GPS latitude
           .size = sizeof(data->latitude),
           .data = (uint8_t *)&data->latitude
          },
          {// [7:4] GPS longitude
           .size = sizeof(data->longitude),
           .data = (uint8_t *)&data->longitude
          },
          {// [9:8] GPS navigation status
           .size = sizeof(data->navstat),
           .data = (uint8_t *)data->navstat
          },
          {// [13:10]
           .size = sizeof(q.w),
           .data = (uint8_t *)&q.w
          },
          {// [17:11]
           .size = sizeof(q.x),
           .data = (uint8_t *)&q.x
          },
          {// [21:18]
           .size = sizeof(q.y),
           .data = (uint8_t *)&q.y
          },
          {// [25:22]
           .size = sizeof(q.z),
           .data = (uint8_t *)&q.z
          },
        }
      };

    // Construct byte array from packet structure
    Packet_asBytes(&packet, message.data, message.length);
  }

  // Send packet comment to LoRa author
  Topic_comment(loraTopic, (uint8_t *)&message);
}

/** @} */
