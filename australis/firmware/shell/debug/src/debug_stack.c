/***********************************************************************************
 * @file        debug_stack.c                                                      *
 * @author      Matt Ricci                                                         *
 * @addtogroup  Shell                   																					 *
 *                                                                                 *
 * @{                                                                              *
 ***********************************************************************************/

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"

// ALLOW FORMATTING
#if INCLUDE_uxTaskGetStackHighWaterMark == 1

#include "stdio.h"
#include "string.h"

#include "tasklist.h"
#include "parser.h"
#include "state.h"
#include "shell.h"

static void Task_exec(UART_t *uart, char *flags);
static void Task_help(UART_t *uart);

static UART_t *pUart;

DEFINE_PROGRAM_HANDLE("task", Task_exec, Task_help)

static void Task_help(UART_t *uart) {
  uart->println(uart, "NAME:");
  uart->println(uart, "\ttask\n");
  uart->println(uart, "USAGE:");
  uart->println(uart, "\ttask print name");
  uart->println(uart, "\ttask print stack [--task-name <name>]\n");
  uart->println(uart, "DESCRIPTION:");
  uart->println(uart, "\tprint name");
  uart->println(uart, "\t    Print the name of every task in the Australis Core list.\n");
  uart->println(uart, "\tprint stack");
  uart->println(uart, "\t    Print the stack high-watermark of each/selected task.\n");
  uart->println(uart, "OPTIONS:");
  uart->println(uart, "\t-t, --task-name");
  uart->println(uart, "\t    Select a task to operate with. Has no effect when used with  `print name`.");
  uart->println(uart, "\t-h, --help");
  uart->println(uart, "\t    Print this help and exit");
}

static void printTaskName(TaskHandle_t task) {
  char *taskName = pcTaskGetName(task);
  pUart->println(pUart, taskName);
}

static void printTaskStack(TaskHandle_t task) {
  unsigned int stackRemaining = uxTaskGetStackHighWaterMark(task);
  char *taskName              = pcTaskGetName(task);
  uint8_t taskNameLength      = strlen(taskName);

  pUart->print(pUart, taskName);
  for (uint8_t i = 0; i < (configMAX_TASK_NAME_LEN - taskNameLength); i++) {
    pUart->send(pUart, ' ');
  }

  uint8_t stackRemainingLength = snprintf(NULL, 0, "%u", stackRemaining);

  char stackStr[stackRemainingLength + 1];
  snprintf(stackStr, stackRemainingLength + 1, "%u", stackRemaining);
  pUart->println(pUart, stackStr);
}

/* ============================================================================================== */
/**
 * @brief
 *
 *
 **
 * ============================================================================================== */
static void Task_exec(UART_t *uart, char *flags) {
  pUart            = uart;

  ArgParser parser = ArgParser_init();
  int argPrintIdx  = parser.addArg(
    &parser, "print", 0, ARG_TYPE_STRING, false
  );
  int argTaskNameIdx = parser.addArg(
    &parser, "--task-name", 't', ARG_TYPE_STRING, false
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
    char *value  = parser.args[argPrintIdx].value;
    State *state = State_getState();

    char str[50] = "";

    // Print name for each task
    if (!strcmp(value, "name")) {
      uart->println(uart, "");
      uart->println(uart, "Task Name       ");
      uart->println(uart, "----------------");
      TaskList_forEach(printTaskName);
    }

    // Print task for each/requested task
    else if (!strcmp(value, "stack")) {
      TaskHandle_t taskHandle;
      uart->println(uart, "");
      uart->println(uart, "Task Name       Stack High Watermark");
      uart->println(uart, "------------------------------------");

      Argument argTaskName = parser.args[argTaskNameIdx];
      char *taskName       = argTaskName.provided ? argTaskName.value : NULL;

      // Print error if requested task does not exist
      if (taskName && (taskHandle = TaskList_getTaskByName(taskName))) {
        printTaskStack(taskHandle);
      }
      // Print stack for task if provided
      else if (taskName) {
        snprintf(str, sizeof(str), "Error: Task `%s` not found\n", taskName);
      }
      // Print stack for all tasks otherwise
      else {
        TaskList_forEach(printTaskStack);
      }
    }

    // Invalid argument
    else {
      snprintf(str, sizeof(str), "Error: invalid argument\n");
    }

    uart->println(uart, str);
  }

  pUart = NULL;
}

#endif

/** @} */
