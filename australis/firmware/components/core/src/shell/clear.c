/***********************************************************************************
 * @file        clear.c                                                            *
 * @author      Matt Ricci                                                         *
 * @addtogroup  Shell                   																					 *
 *                                                                                 *
 * @{                                                                              *
 ***********************************************************************************/

#include "shell.h"
#include "uart.h"

static void Clear_exec(UART_t *, char *);

DEFINE_PROGRAM_HANDLE("clear", Clear_exec, NULL)

/* =============================================================================== */
/**
 * @brief Clear shell window
 **
 * =============================================================================== */
static void Clear_exec(UART_t *uart, char *flags) {
  uart->print(uart, CMD_CLEAR);
}

/** @} */
