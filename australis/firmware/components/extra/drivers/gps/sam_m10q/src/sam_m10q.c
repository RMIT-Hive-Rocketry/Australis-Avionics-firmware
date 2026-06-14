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
#include "math.h"

#include "uart.h"
#include "devicelist.h"
#include "state.h"

#include "FreeRTOS.h"
#include "task.h"

static uint8_t _checksumNMEA(const char *str);




void SAM_M10Q_UART_Interrupt(void) {
  SAM_M10Q_t* gps = (SAM_M10Q_t*)DeviceList_getDeviceHandle(DEVICE_GPS).device ;
  
  USART_Generic_IRQHandler( gps->uart );

  // Notify the task that UART data is available to be read.
  xTaskNotifyFromISR(gps->taskHandle,0,eNoAction,NULL);

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
bool SAM_M10Q_init(SAM_M10Q_t *gps, UART_t *uart, uint32_t baud) {
  gps->uart       = uart;
  gps->setBaud    = SAM_M10Q_setBaud;
  gps->parse      = SAM_M10Q_Parse;
  gps->baud       = baud;
  
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



//(field0[1] == 'P' || field0[1] == 'L' || field0[1] == 'A' || field0[1] == 'B' || \
//               field0[1] == 'I' || field0[1] == 'Q' || field0[1] == 'N')) {
                 
static bool SAM_M10Q_Parse_GLL(SAM_M10Q_t *gps, char* data);
static bool SAM_M10Q_Parse_GGA(SAM_M10Q_t *gps, char* data);


bool SAM_M10Q_Parse(SAM_M10Q_t *gps) {
  static SAM_M10Q_Parse__States_t state = SMS_Idle;
  char byte;
  static char    field0[6];
  static uint8_t field0_i;
  static char    data[96];
  static uint8_t data_i;
  
  bool data_ready = false;

  
  // Grab char.
  while (gps->uart->receive(gps->uart, &byte))
    {
  
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
          if ( ((field0[0] == 'G') && (field0[2] == 'G') && (field0[3] == 'L') && (field0[4] == 'L')) ||
               ((field0[0] == 'G') && (field0[2] == 'G') && (field0[3] == 'G') && (field0[4] == 'A')) ) {
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
        if (data[data_i-2] == '\r' && data[data_i-1] == '\n') {
          state = SMS_Transmission_Complete;
        }

        break;
      }
      
    }

  
  
  if (state == SMS_Transmission_Complete) {
    
    if ((field0[0] == 'G') && (field0[2] == 'G') && (field0[3] == 'L') && (field0[4] == 'L')) {
      data_ready = SAM_M10Q_Parse_GLL(gps, data);
    }
    // Exclude N from second field because it tends to report a maligned packet.
    else if ((field0[0] == 'G') && (field0[1] != 'N') && (field0[2] == 'G') && (field0[3] == 'G') && (field0[4] == 'A')) {
      data_ready = SAM_M10Q_Parse_GGA(gps, data);
    }
    
    state = SMS_Idle;
  }


  
  return data_ready;

}



static bool SAM_M10Q_Parse_GLL(SAM_M10Q_t *gps, char* data) {
  
  char* s_lat     = &data[ 0];
  char* s_NS      = &data[11];
  char* s_lon     = &data[13];
  char* s_EW      = &data[25];
  char* s_status  = &data[37];
  char* s_posMode = &data[39];
  char* s_cs      = &data[41];

  float min_to_deg = 5.0/3.0;
    

  // Latitude conversion.
  uint32_t lat_degrees =                      \
    ((s_lat[0]-'0') * 10) +                   \
    ((s_lat[1]-'0') *  1);
    
  uint32_t lat_minutes_int =                  \
    ((s_lat[2]-'0') * 1000000) +              \
    ((s_lat[3]-'0') *  100000) +              \
    ((s_lat[5]-'0') *   10000) +              \
    ((s_lat[6]-'0') *    1000) +              \
    ((s_lat[7]-'0') *     100) +              \
    ((s_lat[8]-'0') *      10) +              \
    ((s_lat[9]-'0') *       1);

  float lat_minutes = ((float)lat_minutes_int)/10000000.0;

  float lat = (((float)lat_degrees) + (lat_minutes*min_to_deg));

  if (s_NS[0] == 'S') {
    lat *= -1.0;
  }

  gps->sampleData.latitude = lat;

    
  // Longitude conversion.    
  uint32_t lon_degrees =                       \
    ((s_lon[0]-'0') * 100) +                   \
    ((s_lon[1]-'0') *  10) +                   \
    ((s_lon[2]-'0') *   1);
    
  uint32_t lon_minutes_int =                   \
    ((s_lon[ 3]-'0') * 1000000) +              \
    ((s_lon[ 4]-'0') *  100000) +              \
    ((s_lon[ 6]-'0') *   10000) +              \
    ((s_lon[ 7]-'0') *    1000) +              \
    ((s_lon[ 8]-'0') *     100) +              \
    ((s_lon[ 9]-'0') *      10) +              \
    ((s_lon[10]-'0') *       1);

  float lon_minutes = ((float)lon_minutes_int)/10000000.0;

  float lon = (((float)lon_degrees) + (lon_minutes*min_to_deg));
    
  if (s_EW[0] == 'W') {
    lon *= -1.0;
  }
    
  gps->sampleData.longitude = lon;

  // Navstat
  gps->sampleData.navstat[0] = s_status[0];
  gps->sampleData.navstat[1] = s_posMode[0];

  return true;
}



static bool SAM_M10Q_Parse_GGA(SAM_M10Q_t *gps, char* data) {

  // First, check that the packet is actually populated, as the GNSS chip tends
  // to send 'valid' packets that are full of nothing, in particular with GGA
  // messages than others.
  
  for (uint8_t i = 2; (!(data[i-1] == '\r' && data[i] == '\n')); i++) {  

    if (data[i-2] == data[i-1] == data[i] == ',') {
      return false;
    }
    
  }

  

  // Grab altitude
  char* s_alt = &data[47];

  // Find where the decimal place is.
  uint8_t dot = 0;
  while (s_alt[dot] != '.') {
    dot++;
  }

  // Extract the mantissa and fractional components.
  uint32_t alt_mantissa = 0;
  uint32_t mul = pow(10, dot-1);
  for (uint8_t i = 0; i < dot; i++) {
    alt_mantissa += ((s_alt[i]-'0') * mul);
    mul /= 10;
  }

  float alt_fraction = ((float)(s_alt[dot+1]-'0')) * 0.1f;

  // Update state with the altitude reading.

  static float altitude_initial;  
  float altitude = ((float)(alt_mantissa)) + alt_fraction;
  State* state = State_getState();

  // Altitude is distance from ground, not from sea level (given by the SAM M10Q
  // GNSS chip).
  if (altitude_initial < 0.1f) {
    altitude_initial = altitude;
  }

  state->altitude = altitude - altitude_initial;
  
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
