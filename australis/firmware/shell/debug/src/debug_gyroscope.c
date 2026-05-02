/***********************************************************************************
 * @file        debug_gyroscope.c                                                  *
 * @author      Matt Ricci                                                         *
 * @addtogroup  Shell                   																					 *
 *                                                                                 *
 * @{                                                                              *
 ***********************************************************************************/

#include "string.h"
#include "stdio.h"

#include "shell.h"
#include "parser.h"
#include "state.h"
#include "devicelist.h"
#include "gyroscope.h"

static void Gyro_exec(UART_t *uart, char *);

DEFINE_PROGRAM_HANDLE("gyro", Gyro_exec, NULL)

/* =============================================================================== */
/**
 * @brief
 *
 *
 **
 * =============================================================================== */
static void Gyro_exec(UART_t *uart, char *flags) {

  ArgParser parser = ArgParser_init();
  int argPrintIdx  = parser.addArg(
    &parser, "print", 0, ARG_TYPE_BOOL, false
  );
  int argResetIdx = parser.addArg(
    &parser, "reset", 0, ARG_TYPE_BOOL, false
  );
  int argRepeatIdx = parser.addArg(
    &parser, "--repeat", 'r', ARG_TYPE_BOOL, false
  );
  int argVerboseIdx = parser.addArg(
    &parser, "--verbose", 'v', ARG_TYPE_BOOL, false
  );
  int argBiasIdx = parser.addArg(
    &parser, "bias", 0, ARG_TYPE_STRING, false
  );

  char *tokens[MAX_ARGS];
  int numTokens = 0;

  // Tokenize the input string
  char *token = strtok(flags, " ");
  while (token != NULL && numTokens < MAX_ARGS) {
    tokens[numTokens++] = token;
    token               = strtok(NULL, " ");
  }

  // Parse input tokens
  parser.parseArgs(&parser, numTokens, tokens);

  // Early exit with error message
  if (parser.error.status == PARSER_STATUS_ERROR) {
    uart->println(uart, parser.error.msg);
    return;
  }

  Gyro_t *gyro = DeviceList_getDeviceHandle(DEVICE_GYRO).device;

  // Print requested state variable value
  if (parser.args[argPrintIdx].provided) {
    State *state   = State_getState();

    char *terminal = (parser.args[argRepeatIdx].provided) ? "\r" : "\r\n";

  REPEAT:
    if (parser.args[argVerboseIdx].provided) {
      // Print timestamp
      char timestamp[20];
      snprintf(
        timestamp, sizeof(timestamp), "[t=%fs] ",
        state->flightTimeMs / 1000.0f
      );
      uart->print(uart, timestamp);
    }

    char str[50];
    snprintf(
      str, 50, "X: % -10.4f Y: % -10.4f Z: % -10.4f%s",
      gyro->gyroData[0],
      gyro->gyroData[1],
      gyro->gyroData[2],
      terminal
    );
    uart->print(uart, str);

    // Continue until interrupted if specified
    if (parser.args[argRepeatIdx].provided) {
      goto REPEAT;
    }
  }

  // Reset bias parameters
  else if (parser.args[argResetIdx].provided) {
    gyro->bias[0]        = 0;
    gyro->bias[1]        = 0;
    gyro->bias[2]        = 0;
    gyro->gyroData[0]    = 0;
    gyro->gyroData[1]    = 0;
    gyro->gyroData[2]    = 0;
    gyro->rawGyroData[0] = 0;
    gyro->rawGyroData[1] = 0;
    gyro->rawGyroData[2] = 0;
    gyro->rawGyroData[3] = 0;
    gyro->rawGyroData[4] = 0;
    gyro->rawGyroData[5] = 0;
    uart->println(uart, "Reset gyroscope parameters.");
  }

  else {
    uart->println(uart, parser.error.msg);
  }
}

/** @} */
