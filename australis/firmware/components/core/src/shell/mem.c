/***********************************************************************************
 * @file        launch.c                                                           *
 * @author      Matt Ricci                                                         *
 * @addtogroup  Shell                   																					 *
 *                                                                                 *
 * @{                                                                              *
 ***********************************************************************************/

#include "devicelist.h"

#include "string.h"
#include "stdio.h"

#include "dataframe.h"
#include "flash.h"
#include "parser.h"
#include "shell.h"

static void Mem_exec(UART_t *uart, char *);
static void Mem_help(UART_t *uart);

DEFINE_PROGRAM_HANDLE("mem", Mem_exec, Mem_help)

static void Mem_help(UART_t *uart) {
  uart->println(uart, "NAME:");
  uart->println(uart, "\tmem\n");
  uart->println(uart, "USAGE:");
  uart->println(uart, "\tmem erase");
  uart->println(uart, "\tmem read [all | apogee]\n");
  uart->println(uart, "DESCRIPTION:");
  uart->println(uart, "\terase");
  uart->println(uart, "\t    Erase all data written to memory.\n");
  uart->println(uart, "\tread [all]");
  uart->println(uart, "\t    Read binary data from memory.\n");
  uart->println(uart, "OPTIONS:");
  uart->println(uart, "\t-h, --help");
  uart->println(uart, "\t    Print this help and exit");
}

/* =============================================================================== */
/**
 * @brief Execute flash commands on target
 *
 * @details `usbFlashCommandExecute` parses and executes flash related commands
 * according to the flags passed in by the top level command parser.
 * @details Currently implemented commands include:
 * 	- Erase
 * 	- Read all
 **
 * =============================================================================== */
static void Mem_exec(UART_t *uart, char *flags) {
  Flash_t *flash   = DeviceList_getDeviceHandle(DEVICE_FLASH).device;

  ArgParser parser = ArgParser_init();
  int argReadIdx   = parser.addArg(
    &parser, "read", 0, ARG_TYPE_STRING, false
  );
  int argEraseIdx = parser.addArg(
    &parser, "erase", 'r', ARG_TYPE_BOOL, false
  );

  // clang-format off
  int mutexResetRepeatIdx = parser.addMutexGroup(
    &parser, (int[]){
      argReadIdx, 
      argEraseIdx, 
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

  if (parser.args[argReadIdx].provided) {
    // Read all pages and print raw binary
    if (!strcmp(parser.args[argReadIdx].value, "all")) {
      taskENTER_CRITICAL();
      volatile uint8_t pageData[256];
      for (unsigned long i = 0; i < flash->pageCount; i++) {
        flash->readPage(flash, i * 0x100, pageData);
        for (unsigned int j = 0; j < flash->pageSize; j++) {
          uart->send(uart, pageData[j]);
        }
      }
      taskEXIT_CRITICAL();
    }
    // Search memory for apogee event and print results
    else if (!strcmp(parser.args[argReadIdx].value, "apogee")) {

      char str[50];

      uint8_t flashData[259]; // 3-byte overlap + 256-byte page
      uint8_t header[] = {HEADER_EVENT_APOGEE, 0xB0, 0x0B};

      uint8_t *packet  = NULL;

      // Read in first page
      flash->readPage(flash, 0, &flashData[3]);
      memset(flashData, 0, 3); // Clear first 3 bytes for boundary condition

      // Iterate each page
      for (unsigned long i = 0; i < flash->pageCount; i++) {
        snprintf(str, 50, "\rChecking page %ld of %ld", i, flash->pageCount);
        uart->print(uart, str);
        if (i > 0) {
          // After the first page, copy the last 3 bytes from previous
          // into the first 3 bytes of the next
          memcpy(&flashData[0], &flashData[256], 3);
          // Read  the next page beginning after bytes from previous
          flash->readPage(flash, i * flash->pageSize, &flashData[3]);
        }
        // Iterate every byte in page
        for (int j = 0; j < 256; j++) {
          // Write string if event dataframe is found and exit
          if (!memcmp((packet = &flashData[j]), header, 3)) {
            float flightTime = *(uint16_t *)&flashData[j + 3] / 1000.0f;
            snprintf(str, 50, "\r\nApogee at %fs.", flightTime);
            goto EXIT;
          }
        }
      }

      // Write error to string if no event dataframe is found
      snprintf(str, 50, "\r\nNo apogee event was found in memory.");

    EXIT:
      // Print the result
      uart->println(uart, str);
    }

    // Request cannot be fulfilled
    else {
      uart->println(uart, "Error: Requested value for read not recognised.");
    }
  }

  else if (parser.args[argEraseIdx].provided) {
    uart->print(uart, "Clearing flash... ");
    flash->erase(flash);
    uart->println(uart, "Done.");
  }
}

/** @} */
