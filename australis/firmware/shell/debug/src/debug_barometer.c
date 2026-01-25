/***********************************************************************************
 * @file        launch.c                                                           *
 * @author      Matt Ricci                                                         *
 * @addtogroup  Shell                   																					 *
 *                                                                                 *
 * @{                                                                              *
 ***********************************************************************************/

#include "string.h"
#include "stdio.h"

#include "shell.h"
#include "parser.h"
#include "devicelist.h"
#include "barometer.h"

static void Baro_exec(UART_t *uart, char *);

DEFINE_PROGRAM_HANDLE("baro", Baro_exec, NULL)

char str[50];

/* =============================================================================== */
/**
 * @brief
 *
 *
 **
 * =============================================================================== */
static void Baro_exec(UART_t *uart, char *flags) {

  // Initialise parser
  ArgParser parser    = ArgParser_init();
  int SHELL_BARO_READ = parser.addArg(
    &parser, "read", 'r', ARG_TYPE_BOOL, false
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

  Baro_t *baro = DeviceList_getDeviceHandle(DEVICE_BARO).device;
  if (parser.error.status == PARSER_STATUS_OK && parser.args[SHELL_BARO_READ].provided) {
    baro->update(baro);
    snprintf(str, 50, "Ground pressure: %f", baro->groundPress);
    uart->println(uart, str);
    snprintf(str, 50, "Current pressure: %f", baro->press);
    uart->println(uart, str);
    snprintf(str, 50, "Current temperature: %f", baro->temp);
    uart->println(uart, str);
  } else {
    uart->println(uart, parser.error.msg);
  }
}

/** @} */
