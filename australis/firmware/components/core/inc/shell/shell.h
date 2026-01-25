/**
 * @author Matt Ricci
 * @addtogroup System
 */

// ALLOW FORMATTING
#ifndef SHELL_H
#define SHELL_H

#include "FreeRTOS.h"
#include "stdbool.h"
#include "task.h"

#include "topic.h"
#include "uart.h"

// extern Topic *shellTopic;
void Shell_setUart(UART_t *uart);
void pubShellRxInterrupt();

// clang-format off

// Macro for shell program definition boilerplate. The expansion of this will define
// a static global-scope ShellProgramHandle_t pointer to a compound literal struct.
#define DEFINE_PROGRAM_HANDLE(progName, execFunction, helpFunction)              \
static ShellProgramHandle_t __attribute__((section(".shell_" progName), unused)) \
*progHandle = &(ShellProgramHandle_t){                                           \
    .name = progName,                                                            \
    .exec = execFunction,                                                        \
    .help = helpFunction                                                         \
};

// Max allowable characters for name
#define SHELL_PROGRAM_NAME_LENGTH 20

// Max allowable characters for shell string
#define SHELL_MSG_LENGTH 255

// Terminal clear screen control sequence
#define CMD_CLEAR "\033[3J\033[H\033[2J"

// clang-format on

/**
 * @ingroup System
 * @addtogroup Shell
 * @brief Australis shell interface for user interaction
 *        with the system via terminal command line.
 * @{
 */

/**
 * @brief   Struct definition for shell program handle.
 * @details Provides the interface for shell programs.
 * @details All handles must have a name (case-sensitive) that represents the
 *          program as it will be called from the shell interface.
 * @details Additionally, the handle must initialise the exec function pointer.
 *          This defines the program's entry point and is called by the shell.
 */
typedef struct ShellProgramHandle_t {
  char name[SHELL_PROGRAM_NAME_LENGTH]; //!< Program name as referenced by the shell
  void (*exec)(UART_t *, char *);       //!< Program entry point function pointer
  void (*help)(UART_t *);               //!< Program help-string print function
} ShellProgramHandle_t;

void vShellProcess(void *argument);
bool Shell_run(char *programName);
bool Shell_sigint();

/** @} */

#endif
