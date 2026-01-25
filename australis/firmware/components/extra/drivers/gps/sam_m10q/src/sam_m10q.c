/**************************************************************************************************
 * @file        gps.c                                                                             *
 * @author      Matt Ricci                                                                        *
 * @addtogroup  GPS                                                                               *
 * @brief       Brief description of the file's purpose.                                          *
 *                                                                                                *
 * @{                                                                                             *
 **************************************************************************************************/

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
  gps->pollPUBX  = SAM_M10Q_pollPUBX;
  gps->parsePUBX = SAM_M10Q_parsePUBX;
  gps->baud      = baud;

  gps->uart->print(gps->uart, GPS_PUBX_SILENCE);
  gps->setBaud(gps, gps->baud);

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

/* ============================================================================================== */
/**
 * @brief
 *
 * @param
 *
 * @return
 **
 * ============================================================================================== */
void SAM_M10Q_pollPUBX(SAM_M10Q_t *gps) {
  UART_t *uart = gps->uart;
  uart->print(uart, GPS_PUBX_POLL);
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
bool SAM_M10Q_parsePUBX(SAM_M10Q_t *gps, uint8_t *bytes, SAM_M10Q_Data *data) {
  char *string      = (char *)bytes;
  char *const delim = ",";

  char *tokens[SAM_M10Q_PUBX_POSITION_FIELD_COUNT];

  char *token       = strtok(string, ",");
  uint8_t numTokens = 0;
  // Tokenize string data into array
  while (token != NULL && numTokens < SAM_M10Q_PUBX_POSITION_FIELD_COUNT) {
    tokens[numTokens++] = token;             // Store the token
    token               = strtok(NULL, ","); // Get the next token
  }

  // Early exit if first tokens aren't PUBX identifiers
  if (strcmp(tokens[SAM_M10Q_PUBX_POSITION_TOKEN], "$PUBX")
      || strcmp(tokens[SAM_M10Q_PUBX_POSITION_ID], "00")) {
    return false;
  }

  // Copy message tokens to GPS data struct
  strncpy(&data->ns, tokens[SAM_M10Q_PUBX_POSITION_NS], sizeof(data->ns));
  strncpy(&data->ew, tokens[SAM_M10Q_PUBX_POSITION_EW], sizeof(data->ew));
  strncpy(data->time, tokens[SAM_M10Q_PUBX_POSITION_TIME], sizeof(data->time));
  strncpy(data->navstat, tokens[SAM_M10Q_PUBX_POSITION_NAV_STAT], sizeof(data->navstat));

  // Parse coordinates as floats
  data->latitude  = strtof(tokens[SAM_M10Q_PUBX_POSITION_LAT], NULL);
  data->longitude = strtof(tokens[SAM_M10Q_PUBX_POSITION_LONG], NULL);

  if (data->ns == 'S') {
    data->latitude = data->latitude * -1;
  }
  if (data->ew == 'W') {
    data->longitude = data->longitude * -1;
  }

  gps->sampleData = *data;

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
