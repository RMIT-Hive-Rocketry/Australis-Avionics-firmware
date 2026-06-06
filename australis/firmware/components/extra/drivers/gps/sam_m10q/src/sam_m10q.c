/**************************************************************************************************
 * @file        gps.c                                                                             *
 * @author      Matt Ricci                                                                        *
 * @addtogroup  GPS                                                                               *
 * @brief       Brief description of the file's purpose.                                          *
 *                                                                                                *
 * @{                                                                                             *
 **************************************************************************************************/

// README
// This is a temporary solution until the UBX system in the experimental branch
// can be proven to work.

/*******************************************************************************
References
[1] u-blox M10 SPG 5.10 - Standard precision GNSS firmware
    Protocol version 34.10
    https://content.u-blox.com/sites/default/files/u-blox-M10-SPG-5.10_InterfaceDescription_UBX-21035062.pdf
******************************************************************************/

#include "sam_m10q.h"

#include "string.h"
#include "stdio.h"
#include "stdlib.h"


static uint8_t _checksumNMEA(const char *str);

/* ============================================================================================== */
/**
 * @brief
 *
 * @param
 *
 * @return
 **
 * ============================================================================================== */
bool SAM_M10Q_init(SAM_M10Q_t *gps, UART_t *uart, uint32_t baud) {
  gps->uart      = uart;
  gps->setBaud   = SAM_M10Q_setBaud;
  gps->parse     = SAM_M10Q_GLL_Parse;
  gps->baud      = baud;

  // gps->uart->print(gps->uart, GPS_PUBX_SILENCE);
  // gps->setBaud(gps, gps->baud);

  return true;
}

/* ============================================================================================== */
/**
 * @brief
 *
 * @param
 *
 * @return
 **
 * ============================================================================================== */
// clang-format off
void SAM_M10Q_setBaud(SAM_M10Q_t *gps, uint32_t baud) {
  char str1[35]; char str2[35]; char *_str = "$PUBX,41,1,0003,0003,%u,0*%x\r\n";
  snprintf(str1, sizeof(str1), _str, baud, 0);
  snprintf(str2, sizeof(str2), _str, baud, _checksumNMEA(str1));
  gps->uart->print(gps->uart, str2);
  gps->uart->setBaud(gps->uart, baud);
}
// clang-format on


typedef enum {
  SMS_Idle,
  SMS_Field0,
  SMS_Data,
  SMS_Transmission_Complete
} SAM_M10Q_Parse__States_t;



bool SAM_M10Q_GLL_Parse(SAM_M10Q_t *gps) {
  static SAM_M10Q_Parse__States_t state = SMS_Idle;
  char byte;
  char field0[5];
  uint8_t field0_i;
  char data[64];
  uint8_t data_i;

  bool data_ready = false;


  static char HISTORY[512];
  static char HIS_PAST;
  static uint32_t HIS_I = 0;
  
  // Grab char.
  byte = gps->uart->receive(gps->uart);

  if ((HIS_PAST != byte) && (HIS_I < 512)) HISTORY[HIS_I++] = byte;
  HIS_PAST = byte;
  
  // Parsing state machine.
  // Parses according to [1, pp.25]
  switch (state) {

    // Wait for the beginning of a message, marked by a '$'.
  case SMS_Idle:

    if (byte == '$') {
      state = SMS_Field0;
      field0_i = 0;
    }

    break;


    
    // Save the first five bytes. If they a valid zero'th field, then accept it.
  case SMS_Field0:

    field0[field0_i++] = byte;

    if (field0_i > 5) {
      // If this is a GLL message, proceed, otherwise return to idle state.
      // Refer to [1, pp.18] for field[1] comparisons
      if ((field0[0] == 'G') && (field0[2] == 'G') && (field0[3] == 'L') && (field0[4] == 'L') &&
          (field0[1] == 'P' || field0[1] == 'L' || field0[1] == 'A' || field0[1] == 'B' || \
           field0[1] == 'I' || field0[1] == 'Q' || field0[1] == 'N')) {
        state = SMS_Data;
        data_i = 0;
      } else {
        state = SMS_Idle;
      }        
    }
    
    break;


    
    // Collect the data until a CRLF is detected.
  case SMS_Data:

    data[data_i++] = byte;

    // Data was successfully taken.
    if (data[data_i-2] == '\r' && data[data_i-1] == '\r') {
      state = SMS_Transmission_Complete;
    }

    break;
  }


  
  // Fill in the data.
  if (state == SMS_Transmission_Complete) {

    char* s_lat     = &data[ 1];
    char* s_NS      = &data[12];
    char* s_lon     = &data[14];
    char* s_EW      = &data[26];
    char* s_status  = &data[38];
    char* s_posMode = &data[40];
    char* s_cs      = &data[42];

    float min_to_deg = 0.05/3.0;
    

    // Longitude conversion.
    uint32_t lon_degrees =                      \
      ((s_lon[0]-'0') * 10) +                   \
      ((s_lon[1]-'0') *  1);
    
    uint32_t lon_minutes_int =                  \
      ((s_lon[2]-'0') * 1000000) +              \
      ((s_lon[3]-'0') *  100000) +              \
      ((s_lon[5]-'0') *   10000) +              \
      ((s_lon[6]-'0') *    1000) +              \
      ((s_lon[7]-'0') *     100) +              \
      ((s_lon[8]-'0') *      10) +              \
      ((s_lon[9]-'0') *       1);

    float lon_minutes = ((float)lon_minutes_int)/10000.0;

    float lon = (((float)lon_degrees) + (lon_minutes*min_to_deg));

    if (s_NS[0] == 'S') {
      lon *= -1.0;
    }

    gps->sampleData.longitude = lon;

    
    // Latitude conversion.    
    uint32_t lat_degrees =                       \
      ((s_lat[0]-'0') * 100) +                   \
      ((s_lat[1]-'0') *  10) +                   \
      ((s_lat[2]-'0') *   1);
    
    uint32_t lat_minutes_int =                   \
      ((s_lat[ 3]-'0') * 1000000) +              \
      ((s_lat[ 4]-'0') *  100000) +              \
      ((s_lat[ 6]-'0') *   10000) +              \
      ((s_lat[ 7]-'0') *    1000) +              \
      ((s_lat[ 8]-'0') *     100) +              \
      ((s_lat[ 9]-'0') *      10) +              \
      ((s_lat[10]-'0') *       1);

    float lat_minutes = ((float)lat_minutes_int)/10000.0;

    float lat = (((float)lat_degrees) + (lat_minutes*min_to_deg));
    
    if (s_EW[0] == 'W') {
      lat *= -1.0;
    }
    
    gps->sampleData.latitude = lat;


    // Navstat
    gps->sampleData.navstat[0] = s_status[0];
    gps->sampleData.navstat[1] = s_posMode[0];


    
    // End
    state = SMS_Idle;
    data_ready = true;
  }

  
  return data_ready;
}


/* ============================================================================================== */
/**
 * @brief
 *
 * @param
 *
 * @return
 **
 * ============================================================================================== */

static uint8_t _checksumNMEA(const char *str) {
  const char *n = str + 1; // Plus one, skip '$'
  uint8_t chk   = 0;

  while ('*' != *n && '\n' != *n && '\0' != *n) {
    chk ^= (uint8_t)*n;
    n++;
  }

  return chk;
}

/** @} */
