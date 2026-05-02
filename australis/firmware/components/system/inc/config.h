/********************************************
 *			STM32F439 														*
 *			Configuration Options									*
 *			Author: Dr. Glenn Matthews						*
 *			Edited By: Matthew Pulis							*
 *			Header File														*
 ********************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
// ALLOW FORMATTING
#ifndef __MISC_CONFIG_H__
#define __MISC_CONFIG_H__

// External Memory Interface
// #define DATA_IN_ExtSDRAM					// Uncomment this line if an external memory interface is required (SDRAM).
#define SDRAM_BANK_1        0x10

// Clock Configuration - External Crystal
#define HSE_STARTUP_TIMEOUT ((uint16_t)0x0500) /*!< Time out for HSE start up */
#define LSE_STARTUP_TIMEOUT (unsigned int)500

// 168 MHz Core, 25 MHz External
#define HSE_VALUE           (unsigned int)25000000 // Default value of the External oscillator in Hz
#define HSI_VALUE           (unsigned int)16000000 // Value of the internal oscillator in Hz.
#define PLL_M               25
#define PLL_N               336
#define PLL_P               2                      // SYSCLK = PLL_VCO / PLL_P
#define PLL_Q               7                      // USB OTG FS, SDIO and RNG Clock =  PLL_VCO / PLLQ
#define SYSCLK_HSE          (((HSE_VALUE / PLL_M) * PLL_N) / PLL_P)

#define HSE_USED            1

// Used to adjust the frequency of the internal clock
#define HSITRIM             0
#define HSICAL              0

#define HSIPLL_M            16
#define HSIPLL_N            336
#define HSIPLL_P            2 // SYSCLK = PLL_VCO / PLL_P
#define HSIPLL_Q            7 // USB OTG FS, SDIO and RNG Clock =  PLL_VCO / PLLQ
#define SYSCLK_HSI          (((HSI_VALUE / HSIPLL_M) * HSIPLL_N) / HSIPLL_P)

// Processor specific defines to allow access to some of the HAL libraries.
#define STM32F439           1
#define STM32F439xx         1

// Interrupt Vector Table Offset (Uncomment if in RAM)
// #define VECT_TAB_SRAM
#define VECT_TAB_OFFSET     0x00

// Configuration for real-time clock
// #define RTC_ENABLE			0

// Table definitions for peripheral bus prescale values
extern char APBPresc[8];
extern short AHBPresc[16];

#endif
