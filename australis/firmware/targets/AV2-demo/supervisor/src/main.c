/***********************************************************************************
 * @file        main.c                                                             *
 * @author      Matt Ricci                                                         *
 ***********************************************************************************/

#include "stm32f439xx.h"

#include "stdbool.h"
#include "stddef.h"

#include "AustralisCore.h"
#include "gpiopin.h"
#include "tim.h"
#include "rcc.h"

void handleDebounce(GPIOpin_t *sw, GPIOpin_t *led, GPIOpin_t *link, TIM_t *debounce, bool *isHeld) {

  // Update held status for each switch on
  // debounce timer expiry
  if (debounce->pollUpdate(debounce)) {
    *isHeld = !(sw->isSet(sw));
  }

  // Start debounce if necessary
  if (!(sw->isSet(sw)) != *isHeld && !debounce->isRunning(debounce)) {
    debounce->startCounter(debounce);
  }

  // Assign outputs based on state
  if (*isHeld) {
    led->set(led);
    link->set(link);
  } else {
    led->reset(led);
    link->reset(link);
  }
}

int main(void) {

  RCC_START_PERIPHERAL(AHB1, GPIOA);
  RCC_START_PERIPHERAL(AHB1, GPIOB);
  RCC_START_PERIPHERAL(AHB1, GPIOC);
  RCC_START_PERIPHERAL(AHB1, GPIOD);
  RCC_START_PERIPHERAL(APB1, TIM6);
  RCC_START_PERIPHERAL(APB1, TIM7);
  RCC_START_PERIPHERAL(APB2, SYSCFG);

  GPIOpin_t sw1     = GPIOpin_init(GPIOB, GPIO_PIN13, &GPIO_CONFIG_INPUT);
  GPIOpin_t sw2     = GPIOpin_init(GPIOD, GPIO_PIN12, &GPIO_CONFIG_INPUT);
  GPIOpin_t link1   = GPIOpin_init(GPIOC, GPIO_PIN2, NULL);
  GPIOpin_t link2   = GPIOpin_init(GPIOC, GPIO_PIN1, NULL);
  GPIOpin_t led1    = GPIOpin_init(GPIOC, GPIO_PIN0, NULL);
  GPIOpin_t led2    = GPIOpin_init(GPIOA, GPIO_PIN1, NULL);

  TIM_Config timCfg = TIM_CONFIG_DEFAULT;
  timCfg.OPM        = true;

  TIM_t sw1Debounce = TIM_init(TIM6, &timCfg);
  sw1Debounce.setTimingPeriod(&sw1Debounce, 0.020f);

  TIM_t sw2Debounce = TIM_init(TIM7, &timCfg);
  sw2Debounce.setTimingPeriod(&sw2Debounce, 0.020f);

  // Define with initialised state for each switch
  // Pin is held in pull-up so we take the logical inverse
  bool sw1IsHeld = !sw1.isSet(&sw1);
  bool sw2IsHeld = !sw2.isSet(&sw2);

  while (1) {
    handleDebounce(&sw1, &led1, &link1, &sw1Debounce, &sw1IsHeld);
    handleDebounce(&sw2, &led2, &link2, &sw2Debounce, &sw2IsHeld);
  }

  return 0;
}
