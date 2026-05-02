/***********************************************************************************
 * @file        launch.c                                                           *
 * @author      Matt Ricci                                                         *
 * @addtogroup  Shell                   																					 *
 *                                                                                 *
 * @{                                                                              *
 ***********************************************************************************/

#if defined(EXTRA_sx1272) && defined(EXTRA_lora)

#include "AustralisConfig.h"

#include "FreeRTOS.h"
#include "event_groups.h"
#include "groups.h"

#include "stdio.h"
#include "string.h"

#include "parser.h"
#include "shell.h"

#include "lorapub.h"
#include "sx1272.h"

static void SX1272_exec(UART_t *uart, char *flags);
static void SX1272_help(UART_t *uart);

DEFINE_PROGRAM_HANDLE("sx1272", SX1272_exec, SX1272_help)

static void SX1272_help(UART_t *uart) {
  uart->println(uart, "NAME:");
  uart->println(uart, "\tsx1272\n");
  uart->println(uart, "USAGE:");
  uart->println(uart, "\tconfigure\n");
  uart->println(uart, "DESCRIPTION:");
  uart->println(uart, "\tProvides debug access to a SX1272 device associated with the LoRa publication\n");
  uart->println(uart, "\tconfigure");
  uart->println(uart, "\t    Configure options for SX1272 transceiver.\n");
  uart->println(uart, "OPTIONS:");
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
static void SX1272_exec(UART_t *uart, char *flags) {

  ArgParser parser    = ArgParser_init();
  int argOperationIdx = parser.addArg(
    &parser, "configure", 0, ARG_TYPE_STRING, true
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

  // Early exit with error message if parse failed
  if (parser.error.status == PARSER_STATUS_ERROR) {
    uart->println(uart, parser.error.msg);
    return;
  }

  SX1272_t *transceiver = (SX1272_t *)LoRa_getTransceiver();

  // Early exit with error message if transceiver is not set
  if (transceiver == NULL) {
    uart->println(uart, "Error: No transceiver is available on the LoRa topic.");
    return;
  }

  if (parser.args[argOperationIdx].provided) {
    SX1272_Config config = transceiver->config;

    transceiver->updateConfig(transceiver, &config);
  }
}

#endif

/** @} */
