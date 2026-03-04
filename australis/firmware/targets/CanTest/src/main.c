/***********************************************************************************
 * @file        AV2m.c                                                             *
 * @author      Matt Ricci                                                         *
 ***********************************************************************************/

#include "stdint.h"

#include "stm32f439xx.h"

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "portmacro.h"
#include "task.h"

#include "AustralisCore.h"
#include "tasklist.h"
#include "gpiopin.h"
#include "rcc.h"
#include "can.h"

#include "interrupts/interrupts.h"

#include "devices.h"

TaskHandle_t publicationTestHandle;
TaskHandle_t mainTaskHandle;

void initRCC() {
  // Make sure all peripherals we will use are enabled
  RCC_START_PERIPHERAL(AHB1, GPIOA);
  RCC_START_PERIPHERAL(AHB1, GPIOC);
  RCC_START_PERIPHERAL(APB1, CAN1);
  RCC_START_PERIPHERAL(APB1, TIM6);
  RCC_START_PERIPHERAL(APB2, SYSCFG);
}

typedef struct {
  CAN_t bus;
  GPIOpin_t led1;
  GPIOpin_t led2;
} CAN_context;

/* =============================================================================== */
/**
 * @brief
 *
 **
 * =============================================================================== */
void CAN_rxCallback(void *context) {
  CAN_Packet data;

  CAN_context *ctx = context;

  ctx->bus.receive(&ctx->bus, &data);
  ctx->led1.toggle(&ctx->led1);

  // if (data.word[0] % 4 == 0) {
  //   ctx->led2.toggle(&ctx->led2);
  // }
}

/* =============================================================================== */
/**
 * @brief
 *
 **
 * =============================================================================== */
int main() {

  // Initialise RCC
  initRCC();
  

  static CAN_context ctx;

  static CAN_t bus;
  static GPIOpin_t led1;
  static GPIOpin_t led2;

  led1            = GPIOpin_init(LED1_PORT, LED1_PIN, NULL);
  led2            = GPIOpin_init(LED2_PORT, LED2_PIN, NULL);

  GPIO_Config cfg = GPIO_CONFIG_DEFAULT;
  cfg.mode        = GPIO_MODE_AF;
  cfg.afr         = CAN_AF;

  GPIOpin_t txd   = GPIOpin_init(CAN_PORT, CAN_TXD, &cfg);
  GPIOpin_t rxd   = GPIOpin_init(CAN_PORT, CAN_RXD, &cfg);

  bus             = CAN_init(CAN1, NULL);

  ctx.bus         = bus;
  ctx.led1        = led1;
  ctx.led2        = led2;

  //
  static InterruptHandle handle = {
    .context    = (InterruptContext){.callback = CAN_rxCallback, .context = (void *)&ctx},
    .type       = INTERRUPT_CAN,
    .identifier = CAN1_RX0_IRQn,
    .priority   = 9
  };

  Interrupt_registerHandle(handle);

  while (1);

  return 0;
}
