/***********************************************************************************
 * @file        debug_accelerometer.c                                              *
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
#include "accelerometer.h"

static void Accel_exec(UART_t *uart, char *);

DEFINE_PROGRAM_HANDLE("accel", Accel_exec, NULL)

/* =============================================================================== */
/**
 * @brief
 *
 *
 **
 * =============================================================================== */
static void Accel_exec(UART_t *uart, char *flags) {

  ArgParser parser = ArgParser_init();
  int argPrintIdx  = parser.addArg(
    &parser, "print", 0, ARG_TYPE_BOOL, false
  );
  int argRepeatIdx = parser.addArg(
    &parser, "--repeat", 'r', ARG_TYPE_BOOL, false
  );
  int argVerboseIdx = parser.addArg(
    &parser, "--verbose", 'v', ARG_TYPE_BOOL, false
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

  // Print requested state variable value
  if (parser.args[argPrintIdx].provided) {
    Accel_t *accel = DeviceList_getDeviceHandle(DEVICE_ACCEL).device;
    State *state   = State_getState();

    char *terminal = (parser.args[argRepeatIdx].provided) ? "\r\n" : "\r\n";
    char str[80];

  REPEAT:
    taskENTER_CRITICAL();
    accel->update(accel);
    taskEXIT_CRITICAL();

    if (parser.args[argVerboseIdx].provided) {
      // Print timestamp
      char timestamp[20];
      snprintf(
        timestamp, sizeof(timestamp), "[t=%fs] ",
        state->flightTimeMs / 1000.0f
      );
      uart->print(uart, timestamp);
    }

    // Construct debug string
    snprintf(
      str, 80, "X [%d]: %f Y [%d]: %f Z[%d]: %f%s",
      accel->axes[0],
      accel->accelData[0],
      accel->axes[1],
      accel->accelData[1],
      accel->axes[2],
      accel->accelData[2],
      terminal
    );

    // Print resulting output
    uart->print(uart, str);

    // Continue until interrupted if specified
    if (parser.args[argRepeatIdx].provided) {
      goto REPEAT;
    }

  } else {
    uart->println(uart, parser.error.msg);
  }
}

/** @} */
