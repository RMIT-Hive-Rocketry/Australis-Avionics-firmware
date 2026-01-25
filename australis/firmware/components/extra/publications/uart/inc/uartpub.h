// ALLOW FORMATTING
#ifndef UARTPUB_H
#define UARTPUB_H

#include "topic.h"
#include "uart.h"

extern Topic *uartTopic;

void vUartTransmit(void *pvParameters);
void vUartReceive(void *pvParameters);
void UART_setPeripheral(UART_t *peripheral);
void pubUartInterrupt();

#endif
