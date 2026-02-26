// ALLOW FORMATTING
#ifndef UARTPUB_H
#define UARTPUB_H

#include "uart.h"
#include "queue.h"
#include "broadcast_queue.h"

// Public queue for submitting data for transmission.
extern QueueHandle_t Queue_UART_Transmit;
#define QUEUE_UART_LENGTH 32

// Public broadcast queue for those interested in LoRa messages.
extern Broadcast_Queue_t BQueue_UART_Received;


void UART_Startup();

void vUartTransmit(void *pvParameters);
void vUartReceive(void *pvParameters);
void UART_setPeripheral(UART_t *peripheral);
void pubUartInterrupt();

#endif
