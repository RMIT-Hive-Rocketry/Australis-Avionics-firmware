/***********************************************************************************
 * @file        AV2s.c                                                             *
 * @author      Matt Ricci                                                         *
 ***********************************************************************************/

#include "stm32f439xx.h"

#include "AustralisCore.h"
#include "devices.h"
#include "rcc.h"
#include "tasks.h"

void initRCC() {
  // Make sure all peripherals we will use are enabled
  RCC_START_PERIPHERAL(AHB1, GPIOA);
  RCC_START_PERIPHERAL(AHB1, GPIOB);
  RCC_START_PERIPHERAL(AHB1, GPIOC);
  RCC_START_PERIPHERAL(AHB1, GPIOD);
  RCC_START_PERIPHERAL(AHB1, GPIOE);
  RCC_START_PERIPHERAL(AHB1, GPIOF);
  RCC_START_PERIPHERAL(APB2, SPI1);
  RCC_START_PERIPHERAL(APB1, TIM6);
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

int main() {

  // Initialise RCC
  initRCC();

  // Wait 100ms before device init
  delayPostInit();

  // Initialise device drivers
  initDevices();

  // Bring up core
  Australis_init();

  // Initialise extra tasks
  initTasks();

  // Start the core
  //
  // This runs the FreeRTOS scheduler
  // and will never return.
  Australis_startCore();

  return 0;
}
