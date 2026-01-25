/**
 * @author Matt Ricci
 * @addtogroup UART
 * @{
 * @addtogroup GPS
 * @}
 * @todo Add to UART group
 */

// ALLOW FORMATTING
#ifndef GPS_H
#define GPS_H

#include "stdint.h"
#include "stdbool.h"
#include "uart.h"

#define GPS_PUBX_SILENCE                                                       \
  "$PUBX,40,GLL,0,0,0,0,0,0*5C\r\n$PUBX,40,RMC,0,0,0,0,0,0*47\r\n$PUBX,40,"    \
  "GSA,0,0,0,0,0,0*4E\r\n$PUBX,40,GSV,0,0,0,0,0,0*59\r\n$PUBX,40,GGA,0,0,0,0," \
  "0,0*5A\r\n$PUBX,40,VTG,0,0,0,0,0,0*5E\r\n$PUBX,40,GSV,0,0,0,0,0,0*59\r\n"

#define GPS_PUBX_POLL                      "$PUBX,00*33\r\n"

#define SAM_M10Q_PUBX_POSITION_FIELD_COUNT 22
#define SAM_M10Q_PUBX_POSITION_TOKEN       0
#define SAM_M10Q_PUBX_POSITION_ID          1
#define SAM_M10Q_PUBX_POSITION_TIME        2
#define SAM_M10Q_PUBX_POSITION_LAT         3
#define SAM_M10Q_PUBX_POSITION_NS          4
#define SAM_M10Q_PUBX_POSITION_LONG        5
#define SAM_M10Q_PUBX_POSITION_EW          6
#define SAM_M10Q_PUBX_POSITION_NAV_STAT    8

/**
 * @addtogroup GPS
 * @{
 */

typedef struct {
  char time[15];
  float latitude;
  char ns;
  float longitude;
  char ew;
  char altref[15];
  char navstat[2];
} SAM_M10Q_Data;

typedef struct SAM_M10Q {
  UART_t *uart;
  uint32_t baud;
  SAM_M10Q_Data sampleData;
  void (*message)(struct SAM_M10Q *, char *);
  void (*setBaud)(struct SAM_M10Q *gps, uint32_t baud);                         //!<
  void (*pollPUBX)(struct SAM_M10Q *gps);                                       //!<
  bool (*parsePUBX)(struct SAM_M10Q *gps, uint8_t *bytes, SAM_M10Q_Data *data); //!<
} SAM_M10Q_t;

bool SAM_M10Q_init(SAM_M10Q_t *gps, UART_t *uart, uint32_t baud);
void SAM_M10Q_setBaud(SAM_M10Q_t *gps, uint32_t baud);
void SAM_M10Q_pollPUBX(SAM_M10Q_t *gps);
bool SAM_M10Q_parsePUBX(SAM_M10Q_t *gps, uint8_t *bytes, SAM_M10Q_Data *data);

/** @} */
#endif
