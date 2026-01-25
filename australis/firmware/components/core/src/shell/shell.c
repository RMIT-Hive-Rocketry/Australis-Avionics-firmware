/***********************************************************************************
 * @file        shell.c                                                            *
 * @author      Matt Ricci                                                         *
 * @addtogroup  Shell                                                              *
 *                                                                                 *
 * @todo Add commands to buffer to allow managing shell history for frontend       *
 ***********************************************************************************/

#if (coreSHELL_ENABLE == 1)

#include "shell.h"

#include "string.h"

#include "tasklist.h"
#include "devicelist.h"
#include "uart.h"
#include <stdint.h>

extern uint32_t __shell_vector_start;
extern uint32_t __shell_vector_end;

static TaskHandle_t taskHandle;          // Handle of currently active program in shell.
static TaskHandle_t vShellProcessHandle; // Handle of shell processing task

// TODO:
// Add deviceReady flag to driver API to indicate
// when a device struct is initialised and populated
static UART_t *uart;

char *prompt = "(CORE ACTIVE) Australis >: ";

/* =============================================================================== */
/**
 * @brief Executes a shell program in a newly created task.
 *
 * Runs the shell program using the provided task parameters and deletes the task
 * upon completion.
 *
 * @param argument Pointer to the task parameters (ShellTaskParams).
 * @return void
 *
 * =============================================================================== */
void vShellExec(void *argument) {

  Shell_run((char *)argument);
  uart->print(uart, prompt);

  // Remove handle from shell first to prevent
  // nullptr dereference in Shell_clear
  taskHandle = NULL;

  // Delete task and end thread
  vTaskDelete(NULL);
}

/* =============================================================================== */
/**
 * @brief
 *
 * @param argument Pointer to the task parameters .
 * @return void
 *
 * =============================================================================== */
void vShellProcess(void *argument) {
  CREATE_MESSAGE(rxMsg, UART_MSG_LENGTH);

  rxMsg.length = 0; // Reset length first to indicate how much of the buffer is full
  uint32_t rxData;

  vShellProcessHandle = xTaskGetCurrentTaskHandle();

  // Hang task until UART is initialised in device list
  while (!(uart = DeviceList_getDeviceHandle(DEVICE_UART_USB).device));

  char *display  = "\0";
  bool suspended = false;

  for (;;) {
    // Wait for new byte on UART
    xTaskNotifyWait(0, 0, &rxData, portMAX_DELAY);

    // Read in received byte to circular buffer
    rxMsg.data[rxMsg.length++]  = rxData;
    rxMsg.length               %= UART_MSG_LENGTH;

    // Initialise display string with last received character
    display = (char[]){rxData, '\0'};

    // Clear terminal on <Ctrl-c>
    if (rxData == SIGINT) {
      Shell_sigint();
      rxMsg.length = 0; // Reset character index / string length
    }

    // Erase character and move cursor backwards on <BS>
    if (rxData == BACKSPACE || rxData == DEL) {
      // Put backspace on display
      if (rxMsg.length > 1) {
        display = "\b \b";
      }
      rxMsg.length -= (rxMsg.length > 1) ? 2 : 1;
    }

    // Suspend scheduler on <Ctrl-z> outside of shell
    if (rxData == SUBSTITUTE && !suspended) {
      vTaskSuspend(TaskList_getTaskByName("HDataAcq"));
      vTaskSuspend(TaskList_getTaskByName("LDataAcq"));
      // (CORE PAUSED)
      uart->println(uart, "\r\nAustralis core paused.");
      prompt = "(CORE PAUSED) Australis >: ";
      if (taskHandle == NULL) {
        uart->print(uart, prompt);
      }
      rxMsg.length = 0;
      suspended    = true;
    }
    // Resume scheduler on <Ctrl-z> outside of shell
    else if (rxData == SUBSTITUTE && suspended) {
      vTaskResume(TaskList_getTaskByName("HDataAcq"));
      vTaskResume(TaskList_getTaskByName("LDataAcq"));
      // (CORE ACTIVE)
      uart->println(uart, "\r\nAustralis core resumed.");
      prompt = "(CORE ACTIVE) Australis >: ";
      if (taskHandle == NULL) {
        uart->print(uart, prompt);
      }
      rxMsg.length = 0;
      suspended    = false;
    }
    // Print display string otherwise
    else {
      uart->print(uart, display);
    }

    // Send command for execution and reset buffer on <Enter>
    if (rxData == CARRIAGE_RETURN) {
      char *termination = (char *)&rxMsg.data[rxMsg.length - 1];
      *termination      = '\0';
      rxMsg.length      = 0;
      uart->print(uart, "\n");

      // Pass command to shell and begin executing
      xTaskCreate(
        vShellExec, "ShellProgram", 512,
        rxMsg.data, configMAX_PRIORITIES - 6,
        &taskHandle
      );
    }
  }
}

/* =============================================================================== */
/**
 * @brief Executes a shell program by name.
 *
 * Parses the program name and flags, then finds and runs the corresponding shell
 * program.
 *
 * @param shell 			Pointer to the Shell structure containing registered programs.
 * @param programName Name of the program to execute, with optional flags.
 * @return void
 *
 * =============================================================================== */
bool Shell_run(char *programName) {

  // Early exit if uart is not initialised
  if (uart == NULL) {
    return false;
  }

  char *token = strtok((char *)programName, " ");
  char *flags = strchr(token, '\0') + 1;

  // TODO:
  // Make this a forEach function that operates on callback functions that are passed
  // each handle. This should not be publically exposed and instead left private to
  // the Australis core.
  //
  // Register programs in vector to shell
  for (uint32_t *i = (uint32_t *)&__shell_vector_start; i < (uint32_t *)&__shell_vector_end; i++) {
    // Dereference memory location and cast to program handle pointer
    ShellProgramHandle_t *handle = (ShellProgramHandle_t *)*i;
    // Iterate shell vector and execute function from handle with matching name (if any)
    if (!strcmp(handle->name, programName)) {
      handle->exec(uart, flags);
      return true; // Early exit if program is found
    }
  }

  // Print help string if no matching command is found
  uart->print(uart, (char *)programName);
  uart->println(uart, ": command not recognized. Run `help` for a list of available commands");

  return false;
}

/* =============================================================================== */
/**
 * @brief Exit any running task and clear to new line.
 *
 * @details
 **
 * =============================================================================== */
bool Shell_sigint() {

  // Early exit if uart is not initialised
  if (uart == NULL) {
    return false;
  }

  // Delete any running task
  if (taskHandle != NULL) {
    vTaskDelete(taskHandle);
    taskHandle = NULL;
  }

  uart->print(uart, "\r\n");
  uart->print(uart, prompt);

  return true;
}

/* ============================================================================================== */
/**
 * @brief Shell UART Rx complete interrupt handler.
 *
 **
 * ============================================================================================== */
void pubShellRxInterrupt() {
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;

  // Exit if peripheral is not ready
  if (uart == NULL) {
    goto UART_NOT_READY;
  }

  if (uart->interface->SR & USART_SR_RXNE) {
    uint8_t data = uart->interface->DR;
    xTaskNotifyFromISR(vShellProcessHandle, data, eSetValueWithOverwrite, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
  }

UART_NOT_READY:
  return;
}

#endif
