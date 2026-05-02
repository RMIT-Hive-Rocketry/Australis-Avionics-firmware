// ALLOW FORMATTING
#ifndef _DATAFRAME_H
#define _DATAFRAME_H

/* ===================================================================== *
 *                      DATAFRAME HEADER DEFINITIONS                     *
 * ===================================================================== */

// GROUNDSTATION LORA
#define LORA_HEADER_AV_DATA           0x03
#define LORA_HEADER_GPS_DATA          0x04

// AEROBRAKES CAN
#define CAN_HEADER_AEROBRAKES_RETRACT 0x602
#define CAN_HEADER_AEROBRAKES_DATA    0x601

// PAYLOAD CAN
#define CAN_HEADER_PAYLOAD_BASE       0x500
#define CAN_HEADER_PAYLOAD_BARO1      0x501
#define CAN_HEADER_PAYLOAD_BARO2      0x502
#define CAN_HEADER_PAYLOAD_ACCEL      0x503
#define CAN_HEADER_PAYLOAD_STATUS     0x504

// FLASH
#define HEADER_ID_Pos                 0x06
#define HEADER_LENGTH_Pos             0x00
#define HEADER_EVENT_SUB_ID_Pos       0x04

#define HEADER_HIGHRES_ID             0x01
#define HEADER_HIGHRES_LENGTH         0x14
#define HEADER_HIGHRES                (HEADER_HIGHRES_ID << HEADER_ID_Pos) | HEADER_HIGHRES_LENGTH
#define HEADER_LOWRES_ID              0x02
#define HEADER_LOWRES_LENGTH          0x0A
#define HEADER_LOWRES                 (HEADER_LOWRES_ID << HEADER_ID_Pos) | HEADER_LOWRES_LENGTH

#define HEADER_EVENT_ID               0x03
#define HEADER_EVENT_LENGTH           0x05
#define HEADER_EVENT_LAUNCH_ID        0x00
#define HEADER_EVENT_COAST_ID         0x01
#define HEADER_EVENT_APOGEE_ID        0x02
#define HEADER_EVENT_DESCENT_ID       0x03

#define HEADER_EVENT_LAUNCH           (HEADER_EVENT_ID << HEADER_ID_Pos | HEADER_EVENT_LAUNCH_ID << HEADER_EVENT_SUB_ID_Pos | HEADER_EVENT_LENGTH)
#define HEADER_EVENT_COAST            (HEADER_EVENT_ID << HEADER_ID_Pos | HEADER_EVENT_COAST_ID << HEADER_EVENT_SUB_ID_Pos | HEADER_EVENT_LENGTH)
#define HEADER_EVENT_APOGEE           (HEADER_EVENT_ID << HEADER_ID_Pos | HEADER_EVENT_APOGEE_ID << HEADER_EVENT_SUB_ID_Pos | HEADER_EVENT_LENGTH)
#define HEADER_EVENT_DESCENT          (HEADER_EVENT_ID << HEADER_ID_Pos | HEADER_EVENT_DESCENT_ID << HEADER_EVENT_SUB_ID_Pos | HEADER_EVENT_LENGTH)

#endif
