/***********************************************************************************
 * @file        AV2m.c                                                             *
 * @author      Matt Ricci                                                         *
 ***********************************************************************************/

#include <FreeRTOS.h>
#include <FreeRTOSConfig.h>
#include <portmacro.h>
#include <projdefs.h>
#include <stdint.h>
#include <task.h>

#include "cmsis_gcc.h"
#include "stm32f439xx.h"

#include "devices.h"
#include "gpiopin.h"
#include "lora.h"
#include "rfm95.h"
#include "sx1272.h"
#include "spi.h"

#include "tasklist.h"
#include "rcc.h"

GPIOpin_t transmitLed;

void initRCC() {
  // Make sure all peripherals we will use are enabled
  RCC_START_PERIPHERAL(AHB1, GPIOA);
  RCC_START_PERIPHERAL(AHB1, GPIOB);
  RCC_START_PERIPHERAL(AHB1, GPIOC);
  RCC_START_PERIPHERAL(AHB1, GPIOD);
  RCC_START_PERIPHERAL(AHB1, GPIOE);
  RCC_START_PERIPHERAL(AHB1, GPIOF);
  RCC_START_PERIPHERAL(AHB1, GPIOG);
  RCC_START_PERIPHERAL(AHB1, GPIOH);
  RCC_START_PERIPHERAL(APB1, SPI3);
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

void vHeartbeatBlink(void *argument) {

  GPIOpin_t heartbeatLED = GPIOpin_init(LED1_PORT, LED1_PIN, NULL);

  heartbeatLED.reset(&heartbeatLED);

  for (;;) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    vTaskDelayUntil(&xLastWakeTime, 675);

    heartbeatLED.toggle(&heartbeatLED);
    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(130));
    heartbeatLED.toggle(&heartbeatLED);
    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(130));
    heartbeatLED.toggle(&heartbeatLED);
    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(125));
    heartbeatLED.toggle(&heartbeatLED);
    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(125));
  }
}

void vMainTask(void *argument) {
  (void)argument;

  // SPI pin configuration
  GPIO_Config spiPinConfig = GPIO_CONFIG_DEFAULT;
  spiPinConfig.mode        = GPIO_MODE_AF;
  spiPinConfig.afr         = LORA_SPI_AF;

  // Initialise SCK/SDI/SDO pins
  GPIOpin_t loraSCK = GPIOpin_init(LORA_SPI_PORT, LORA_SPI_SCK, &spiPinConfig);
  GPIOpin_t loraSDI = GPIOpin_init(LORA_SPI_PORT, LORA_SPI_SDI, &spiPinConfig);
  GPIOpin_t loraSDO = GPIOpin_init(LORA_SPI_PORT, LORA_SPI_SDO, &spiPinConfig);

  // Initialise SPI interface
  static SPI_t spiLora;
  SPI_Config spiLoraConfig = SPI_CONFIG_DEFAULT; // Using default settings as base
  spiLoraConfig.CPHA       = SPI_CPHA_FIRST;     // Begin on first clock edge
  spiLoraConfig.CPOL       = SPI_CPOL0;          // Idle clock low
  spiLora                  = SPI_init(LORA_SPI_INTERFACE, &spiLoraConfig);

  // ==========================================================================
  // LORA
  //
  // LoRa transceiver for external wireless communicatons. Can be configured to
  // either receive or transmit data.

  GPIOpin_t loraCS = GPIOpin_init(LORA_CS, NULL);

  static RFM95_t lora;
  RFM95_init(
    &lora,
    &spiLora,
    loraCS,
    NULL
  );

  uint8_t x = 0;

  for (;;) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(500));

    lora.base.transmit(&lora.base, &x, 1);
    transmitLed.set(&transmitLed);
    x++;
  }
}

int main() {

  // Initialise RCC
  initRCC();

  // Wait 100ms before device init
  delayPostInit();

  transmitLed = GPIOpin_init(LED2_PORT, LED2_PIN, NULL);

  xTaskCreate(
    vMainTask,
    "main",
    512,
    NULL,
    configMAX_PRIORITIES - 2,
    TaskList_new()
  );

  xTaskCreate(vHeartbeatBlink, "HeartbeatBlink", 128, NULL, configMAX_PRIORITIES - 1, TaskList_new());

  __disable_irq();
  NVIC_SetPriority(EXTI1_IRQn, 9);
  NVIC_EnableIRQ(EXTI1_IRQn);
  EXTI->RTSR        |= 0x02;
  EXTI->IMR         |= 0x02;
  SYSCFG->EXTICR[0] &= ~0xF0;
  SYSCFG->EXTICR[0]  = 0x30;
  __enable_irq();

  // Start the scheduler
  vTaskStartScheduler();

  return 0;
}

void EXTI1_IRQHandler(void) {
  EXTI->PR |= (0x02);
  transmitLed.reset(&transmitLed);
}
