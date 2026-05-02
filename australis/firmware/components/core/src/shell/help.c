/***********************************************************************************
 * @file        help.c                                                             *
 * @author      Matt Ricci                                                         *
 * @addtogroup  Shell                   																					 *
 *                                                                                 *
 * @{                                                                              *
 ***********************************************************************************/

#include "stdint.h"

#include "string.h"

#include "shell.h"

static void Help_exec(UART_t *, char *);
extern uint32_t __shell_vector_start;
extern uint32_t __shell_vector_end;

DEFINE_PROGRAM_HANDLE("help", Help_exec, NULL)

/* =============================================================================== */
/**
 * @brief Wrapper for shell help function
 **
 * =============================================================================== */
static void Help_exec(UART_t *uart, char *flags) {

  for (uint32_t *i = (uint32_t *)&__shell_vector_start; i < (uint32_t *)&__shell_vector_end; i++) {
    ShellProgramHandle_t *handle = (ShellProgramHandle_t *)*i;
    if (!strcmp(handle->name, flags)) {
      if (handle->help == NULL) {
        uart->println(uart, "No help documentation found.");
      } else {
        handle->help(uart);
      }
      return;
    }
  }

  uart->println(uart, "Use `help [name]` for more information on a specific command");
  uart->println(uart, "The following commands are currently available:");
  for (uint32_t *i = (uint32_t *)&__shell_vector_start; i < (uint32_t *)&__shell_vector_end; i++) {
    ShellProgramHandle_t *handle = (ShellProgramHandle_t *)*i;
    if (strcmp(handle->name, "")) {
      uart->print(uart, ":");
      uart->print(uart, handle->name);
      uart->print(uart, "\n\r");
    }
  }
  uart->println(uart, "Use <Ctrl+z> to toggle the Australis core between paused and active states.");
  uart->println(uart, "Use <Ctrl+c> to kill an active command.");
}

/** @} */
