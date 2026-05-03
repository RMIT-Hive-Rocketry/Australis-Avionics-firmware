/*******************************************************************************
sam_m10q.h
Driver for the SAM_M10Q chip, used for GPS data.

SAM_M10Q is used for retrieving GPS data. It is a complex chip with its own CPU
 and firmware.
Communication is done over UART according to the default settings of the SAM_M10Q
 chip. We use the UBX protocol, because NMEA/PUBX requires heavy string
 processing which is error prone, especially in C, while UBX is a fixed binary
 format.
The datasheet used for UBX protocol specification is "u-blox M10 SPG 5.10", with
 the document number "UBX-21035062 - R03"
*******************************************************************************/

#ifndef GPS_H
#define GPS_H

#include "stdint.h"
#include "stdbool.h"
#include "uart.h"

// Definition of the SAM_M10Q device.
typedef struct SAM_M10Q {
  UART_t *uart;
  GPIOpin_t _reset;
  GPIOpin_t _tx;
  GPIOpin_t _rx;
} SAM_M10Q_t;




// Essential Frame Structures

typedef struct {
  uint8_t class;
  uint8_t class_id;
} SAM_M10Q_ID_t

typedef struct {
  uint8_t ck_a;
  uint8_t ck_b;
} UBX_Checksum_t;

// Identifies a group of message IDs.
// Not all are implemented; add as necessary.
typedef enum {
  UBX_MSG_ACK = 0x05,
  UBX_MSG_CFG = 0x06,
  UBX_MSG_NAV = 0x01,
} UBX_Message_Class;


// Message identifier.
// Not all are implemented; add as necessary.
typedef enum : SAM_M10Q_ID_t {
  // UBX_FAILED_MESSAGE is an unofficial ID we use to indicate a bad packet.
  UBX_FAILED_MESSAGE = { .class = 0xFF,        .class_id = 0xFF };
  UBX_ACK_ACK        = { .class = UBX_MSG_ACK, .class_id = 0x01 };
  UBX_ACK_NAK        = { .class = UBX_MSG_ACK, .class_id = 0x00 };
  UBX_CFG_VALSET     = { .class = UBX_MSG_CFG, .class_id = 0x8a };
  UBX_NAV_POSLLH     = { .class = UBX_MSG_NAV, .class_id = 0x02 };
} UBX_Message_ID_t;

// A complete UBX frame.
typedef struct {
  uint8_t sync[2] : { 0xb5, 0x62 };
  UBX_Message_ID_t id;
  uint16_t length;
  uint8_t payload[28]; // Length should be the maximum expected frame size.
  UBX_Checksum_t checksum;
} UBX_Frame_t;


// Payload Structures
// For messages, there will be two structures.
//  - The actual payload return structure, one-to-one with the datasheet.
//  - The post-processed data structure.
// They are distinguished to prevent confusion.


typedef struct {
  uint32_t iTOW;
  int32_t lon;
  int32_t lat;
  int32_t height;
  int32_t hMSL;
  uint32_t hAcc;
} UBX_Payload_NAV_POSLLH_t;

typedef struct {
  double longitude;
  double latitude;
  double accuracy;
} GPS_Coordinates_t;



typedef struct {
  uint8_t version;
  uint8_t layers;
  uint16_t _reserved0;
  uint8_t* config; // An arbitrary length list of key-value pairs.
} UBX_Payload_CFG_SETVAL_t;




typedef enum {
  
} UBX_Configurations_t;


// FUNCTIONS


bool
SAM_M10Q_Receive(UBX_Frame_t* frame);



bool
SAM_M10Q_Init();




/** @} */
#endif
