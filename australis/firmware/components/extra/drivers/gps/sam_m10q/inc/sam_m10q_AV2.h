//#ifdef FLIGHT_COMPUTER_AV2

#include "stdbool.h"

#define GPS_INTERFACE         USART3                        // GPS on UART3
#define GPS_PORT              GPIOD                         // UART pins on GPIOD
#define GPS_TX_PIN            GPIO_PIN8                     // GPS Tx on PD8
#define GPS_RX_PIN            GPIO_PIN9                     // GPS Rx on PD9
#define GPS_RESET_PORT        GPIOD                         // GPS reset pin on GPIOD
#define GPS_RESET_PIN         GPIO_PIN13                    // Reset pin PD13
#define GPS_BAUD_STARTUP      9600                         // 115200bps baud rate
#define GPS_BAUD_CONFIGURE    38400                         // 115200bps baud rate

//#endif
