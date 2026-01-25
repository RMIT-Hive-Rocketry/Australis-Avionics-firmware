/***********************************************************************************
 * @file        debug_state.c                                                      *
 * @author      Matt Ricci                                                         *
 * @addtogroup  Shell                   																					 *
 *                                                                                 *
 * @{                                                                              *
 ***********************************************************************************/

#include "AustralisConfig.h"

#include "FreeRTOS.h"
#include "event_groups.h"
#include "groups.h"

#include "stdio.h"
#include "string.h"

#include "parser.h"
#include "state.h"
#include "shell.h"

static void State_exec(UART_t *uart, char *flags);
static void State_help(UART_t *uart);

DEFINE_PROGRAM_HANDLE("state", State_exec, State_help)

extern EventGroupHandle_t xTaskEnableGroup;

static void State_help(UART_t *uart) {
  uart->println(uart, "NAME:");
  uart->println(uart, "\tstate\n");
  uart->println(uart, "USAGE:");
  uart->println(uart, "\tstate print <state_variable> [--repeat]");
  uart->println(uart, "\tstate reset\n");
  uart->println(uart, "DESCRIPTION:");
  uart->println(uart, "\tprint");
  uart->println(uart, "\t    Print the value of a state variable to the shell output.\n");
  uart->println(uart, "\treset");
  uart->println(uart, "\t    Reset the global state to default and clear event groups.\n");
  uart->println(uart, "OPTIONS:");
  uart->println(uart, "\t-r, --repeat");
  uart->println(uart, "\t    Continue to print variable until interrupted. Can only be used with `print`");
  uart->println(uart, "\t-h, --help");
  uart->println(uart, "\t    Print this help and exit");
}

/* ============================================================================================== */
/**
 * @brief
 *
 *
 **
 * ============================================================================================== */
static void State_exec(UART_t *uart, char *flags) {

  ArgParser parser = ArgParser_init();
  int argResetIdx  = parser.addArg(
    &parser, "reset", 0, ARG_TYPE_BOOL, false
  );
  int argPrintIdx = parser.addArg(
    &parser, "print", 0, ARG_TYPE_STRING, false
  );
  int argRepeatIdx = parser.addArg(
    &parser, "--repeat", 'r', ARG_TYPE_BOOL, false
  );
  int argVerboseIdx = parser.addArg(
    &parser, "--verbose", 'v', ARG_TYPE_BOOL, false
  );

  // clang-format off
  int mutexResetPrintIdx = parser.addMutexGroup(
    &parser, (int[]){
      argResetIdx, 
      argPrintIdx, 
    }, 2
  );
  int mutexResetRepeatIdx = parser.addMutexGroup(
    &parser, (int[]){
      argResetIdx, 
      argRepeatIdx, 
    }, 2
  );
  // clang-format on

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

  // Reset state variables to initial value
  if (parser.args[argResetIdx].provided) {
    xEventGroupClearBits(xTaskEnableGroup, 0xFF);
    uart->println(uart, "Reset state parameters.");
    State_init();
  }

  // Print requested state variable value
  if (parser.args[argPrintIdx].provided) {
    char *value    = parser.args[argPrintIdx].value;
    State *state   = State_getState();

    char *terminal = (parser.args[argRepeatIdx].provided) ? "\r" : "\r\n";
    char str[50];

  REPEAT:
    // Print `flightState` state variable
    if (!strcmp(value, "flightState")) {
      switch (state->flightState) {
      case PRELAUNCH:
        snprintf(str, sizeof(str), "State: PRELAUNCH%s", terminal);
        break;
      case LAUNCH:
        snprintf(str, sizeof(str), "State: LAUNCH%s", terminal);
        break;
      case COAST:
        snprintf(str, sizeof(str), "State: COAST%s", terminal);
        break;
      case APOGEE:
        snprintf(str, sizeof(str), "State: APOGEE%s", terminal);
        break;
      case DESCENT:
        snprintf(str, sizeof(str), "State: DESCENT%s", terminal);
        break;
      }
    }

    // Print `altitude` state variable
    else if (!strcmp(value, "altitude")) {
      snprintf(
        str, sizeof(str), "Altitude: %fm%s",
        state->altitude,
        terminal
      );
    }

    // Print `velocity` state variable
    else if (!strcmp(value, "velocity")) {
      snprintf(
        str, sizeof(str), "Velocity: %fm/(s^2)%s",
        state->velocity,
        terminal
      );
    }

    // Print `tilt` state variable
    else if (!strcmp(value, "tilt")) {
      snprintf(
        str, sizeof(str), "Tilt: %f degrees%s",
        state->tilt,
        terminal
      );
    }

    // Print `flightTimeMs` state variable
    else if (!strcmp(value, "flightTimeMs")) {
      snprintf(
        str, sizeof(str), "Flight time: %ums / %fs%s",
        state->flightTimeMs,
        state->flightTimeMs / 1000.0f,
        terminal
      );
    }

    // Requested state variable not found
    else {
      snprintf(str, sizeof(str), "Error: `%s` is not a state variable\n", value);
    }

    // Repeat if flag is set
    if (parser.args[argVerboseIdx].provided) {
      // Print timestamp
      char timestamp[20];
      snprintf(
        timestamp, sizeof(timestamp), "[t=%fs] ",
        state->flightTimeMs / 1000.0f
      );
      uart->print(uart, timestamp);
    }

    // Print resulting output
    uart->print(uart, str);

    // Continue until interrupted if specified
    if (parser.args[argRepeatIdx].provided) {
      goto REPEAT;
    }
  }
}

/** @} */
