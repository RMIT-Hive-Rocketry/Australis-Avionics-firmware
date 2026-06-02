/*******************************************************************************
sam_m10q.h
Driver for the SAM_M10Q chip, used for GPS data.

SAM_M10Q is used for retrieving GPS data. It is a complex chip with its own CPU
 and firmware.
Communication is done over UART according to default settings of the SAM_M10Q
 chip. We use the UBX protocol as NMEA/PUBX requires heavy string processing,
 which is error prone especially in C, while UBX is a fixed binary format.
*******************************************************************************/

/*******************************************************************************
References
[1] u-blox M10 SPG 5.10 - Standard precision GNSS firmware
    Protocol version 34.10
    https://content.u-blox.com/sites/default/files/u-blox-M10-SPG-5.10_InterfaceDescription_UBX-21035062.pdf
[2] SAM-M10Q - Standard precision GNSS antenna module
    Integration Manual
    https://content.u-blox.com/sites/default/files/documents/SAM-M10Q_IntegrationManual_UBX-22020019.pdf
[3] SAM-M10Q - u-blox M10 standard precision GNSS antenna module
    Data sheet
    https://content.u-blox.com/sites/default/files/documents/SAM-M10Q_DataSheet_UBX-22013293.pdf
******************************************************************************/

#ifndef SAM_M10Q_H
#define SAM_M10Q_H

#include "sam_m10q_AV2.h"

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "gpiopin.h"
#include "uart.h"


// Post-processed data from UBX_Payload_NAV_POSLLH_t
typedef struct {
  float longitude;
  float latitude;
  float accuracy;
} GPS_Coordinates_t;


typedef enum {
  SAM_M10Q_Frame_State__Sync_Wait,
  SAM_M10Q_Frame_State__Incoming,
  SAM_M10Q_Frame_State__Complete
} SAM_M10Q_Frame_State_t;




// Essential Frame Structures
typedef struct __attribute__((packed)) {
  uint8_t class;
  uint8_t id;
} UBX_Message_t;


// Fletcher's checksum algorithm [1, pp.44]
typedef struct __attribute__((packed)) {
  uint8_t ck_a;
  uint8_t ck_b;
} UBX_Checksum_t;


// Message identifiers [1, pp.46-48]
extern const UBX_Message_t UBX_ACK_ACK;
extern const UBX_Message_t UBX_ACK_NAK;
extern const UBX_Message_t UBX_CFG_VALSET;
extern const UBX_Message_t UBX_NAV_POSLLH;


// A complete UBX frame.
typedef struct __attribute__((packed)) {
  uint8_t sync[2];
  UBX_Message_t message;
  uint16_t length;
  uint8_t payload[28]; // Byte offset 6. Length should be the maximum expected frame size.
  UBX_Checksum_t checksum;
} UBX_Frame_t;

#define PAYLOAD_BYTE_OFFSET 6




// Definition of the SAM_M10Q device.
typedef struct SAM_M10Q {
  
  UART_t uart;
  GPS_Coordinates_t coordinates;

  // Pin outputs related to GPS.
  GPIOpin_t _reset;
  GPIOpin_t _tx;
  GPIOpin_t _rx;

  // Flags to indicate communication status.
  SAM_M10Q_Frame_State_t _message_state;
  uint16_t _message_position;
  UBX_Frame_t _message_frame;
  
} SAM_M10Q_t;

extern SAM_M10Q_t* gps;



// Payload binary format [1, pp.97]
typedef struct {
  uint32_t iTOW;
  int32_t lon;
  int32_t lat;
  int32_t height;
  int32_t hMSL;
  uint32_t hAcc;
} UBX_Payload_NAV_POSLLH_t;



// Payload binary format [1, pp.55]
typedef struct __attribute__((packed)) {
  uint8_t version;
  uint8_t layers;
  uint8_t _reserved0[2];
  uint8_t config[28]; // An arbitrary length list of key-value pairs.
  size_t _config_length;
} UBX_Payload_CFG_VALSET_t;


typedef enum : uint32_t {

  // Message outputs which are on SAM_M10Q startup turned on, that we want to
  // disable [1, pp.164].
  CFG_MSGOUT_NMEA_ID_GGA_I2C   = 0x209100ba,
    CFG_MSGOUT_NMEA_ID_GGA_SPI   = 0x209100be,
    CFG_MSGOUT_NMEA_ID_GGA_UART1 = 0x209100bb,
    CFG_MSGOUT_NMEA_ID_GLL_I2C   = 0x209100c9,
    CFG_MSGOUT_NMEA_ID_GLL_SPI   = 0x209100cd,
    CFG_MSGOUT_NMEA_ID_GLL_UART1 = 0x209100ca,
    CFG_MSGOUT_NMEA_ID_GSA_I2C   = 0x209100bf,
    CFG_MSGOUT_NMEA_ID_GSA_SPI   = 0x209100c3,
    CFG_MSGOUT_NMEA_ID_GSA_UART1 = 0x209100c0,
    CFG_MSGOUT_NMEA_ID_GSV_I2C   = 0x209100c4,
    CFG_MSGOUT_NMEA_ID_GSV_SPI   = 0x209100c8,
    CFG_MSGOUT_NMEA_ID_GSV_UART1 = 0x209100c5,
    CFG_MSGOUT_NMEA_ID_RMC_I2C   = 0x209100ab,
    CFG_MSGOUT_NMEA_ID_RMC_SPI   = 0x209100af,
    CFG_MSGOUT_NMEA_ID_RMC_UART1 = 0x209100ac,
    CFG_MSGOUT_NMEA_ID_VTG_I2C   = 0x209100b0,
    CFG_MSGOUT_NMEA_ID_VTG_SPI   = 0x209100b4,
    CFG_MSGOUT_NMEA_ID_VTG_UART1 = 0x209100b1,


    // Messages we wish to enable.
    CFG_MSGOUT_UBX_NAV_POSLLH_UART1 = 0x2091002a,
    
  
    // CFG-UART1
    UBX_CFG_UART1_BAUDRATE = 0x40520001, // [1, pp.155]
  
} UBX_Configuration_Key_t;


typedef struct __attribute__((packed)) {
  UBX_Configuration_Key_t key;
  //uint8_t value[8];
  uint8_t value;
  size_t  value_size;
} UBX_Configuration_t;


// FUNCTIONS


bool
SAM_M10Q_Receive();



bool
SAM_M10Q_Init();


typedef enum {
  Code__UBX_Configure__Success,
  Code__UBX_Configure__Payload_Size_Exceeded
} Code__UBX_Configure_t;



bool
UBX_Configure(UBX_Configuration_t config);


void
UBX_Configure_Default();

#endif /* SAM_M10Q_H */
