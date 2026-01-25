/***********************************************************************************
 * @file        AV2m.c                                                             *
 * @author      Matt Ricci                                                         *
 ***********************************************************************************/

#include "stm32f439xx.h"

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "portmacro.h"
#include "projdefs.h"
#include "task.h"

#include "interrupts/interrupts.h"
#include "AustralisCore.h"
#include "tasklist.h"
#include "devices.h"
#include "gpiopin.h"
#include "rcc.h"
#include "can.h"

void initRCC() {
  // Make sure all peripherals we will use are enabled
  RCC_START_PERIPHERAL(AHB1, GPIOA);
  RCC_START_PERIPHERAL(AHB1, GPIOC);
  RCC_START_PERIPHERAL(APB1, TIM6);
  RCC_START_PERIPHERAL(APB1, CAN1);
  RCC_START_PERIPHERAL(APB2, SYSCFG);
}

void delayPostInit() {
  // Delay to ensure time for device POR
  TIM6->ARR &= ~TIM_ARR_ARR; // Clear ARR
  TIM6->PSC &= ~TIM_PSC_PSC; // Clear PSC

  TIM6->PSC |= 83;           // TIM6 clock = f_ck/(PSC + 1) = 42MHz/83 = ~500kHz
  TIM6->ARR |= 49999;        // Auto reload set for 49999 x (1/500kHz) = 100ms

  TIM6->EGR  = TIM_EGR_UG;
  TIM6->SR  &= ~TIM_SR_UIF;

  TIM6->CR1 |= (TIM_CR1_ARPE | TIM_CR1_OPM);
  TIM6->CR1 |= TIM_CR1_CEN; // Enable timer
  while (!(TIM6->SR & TIM_SR_UIF));
  TIM6->SR &= ~TIM_SR_UIF;
}

void EXTI_interruptCallback(void *context) {
  ((GPIOpin_t *)context)->toggle(context);
  EXTI->PR |= EXTI_PR_PR12;
}

void vCanTransmit(void *argument) {

  static CAN_t bus;
  static GPIOpin_t led1;
  static GPIOpin_t led2;

  led1               = GPIOpin_init(LED1_PORT, LED1_PIN, NULL);
  led2               = GPIOpin_init(LED2_PORT, LED2_PIN, NULL);

  GPIO_Config cfg    = GPIO_CONFIG_DEFAULT;
  cfg.mode           = GPIO_MODE_AF;
  cfg.afr            = CAN_AF;

  GPIOpin_t txd      = GPIOpin_init(CAN_PORT, CAN_TXD, &cfg);
  GPIOpin_t rxd      = GPIOpin_init(CAN_PORT, CAN_RXD, &cfg);

  bus                = CAN_init(CAN1, NULL);

  EXTI->RTSR        |= EXTI_RTSR_TR12;
  EXTI->IMR         |= EXTI_IMR_IM12;
  SYSCFG->EXTICR[3] &= SYSCFG_EXTICR4_EXTI12_Msk;
  SYSCFG->EXTICR[3] |= SYSCFG_EXTICR4_EXTI12_PD;

  /////////////////////////////////

  static InterruptHandle handle = {
    .context    = (InterruptContext){.callback = EXTI_interruptCallback, .context = &led2},
    .type       = INTERRUPT_EXTI,
    .identifier = EXTI15_10_IRQn,
    .priority   = 9
  };

  Interrupt_registerHandle(handle);

  /////////////////////////////////

  CAN_Data data = {.id = 0x502, .length = 2, .data = {0x00, 0}};

  for (;;) {
    TickType_t xLastWakeTime = xTaskGetTickCount();

    bus.transmit(&bus, &data);
    data.data[0]++;

    led1.toggle(&led1);

    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(500));
  }
}

int main() {

  // Initialise RCC
  initRCC();

  // Wait 100ms before device init
  delayPostInit();

  xTaskCreate(vCanTransmit, "CanTransmit", 512, NULL, configMAX_PRIORITIES - 1, TaskList_new());

  // Start the core
  //
  // This runs the FreeRTOS scheduler
  // and will never return.
  Australis_startCore();

  return 0;
}
