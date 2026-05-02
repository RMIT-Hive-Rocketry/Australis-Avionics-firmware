/**
 * @author Matt Ricci
 * @addtogroup API
 * @{
 * @addtogroup UART
 * @}
 */

// ALLOW FORMATTING
#ifndef UART_H
#define UART_H

#include "stdbool.h"
#include "stm32f439xx.h"

#define UART_PARITY_DISABLE 0x400
#define UART_SYNC_DISABLE   0xE00
#define UART_FLOW_DISABLE   0x300
#define UART_ENABLE         0x2000
#define UART_RX_ENABLE      0x0004
#define UART_TX_ENABLE      0x0008
#define UART_AF7            0x07
#define UART_AF8            0x08

#define UART_CR1_RESERVED   (0x01 << 14)

#define SIGINT              0x03
#define BACKSPACE           0x08
#define LINE_FEED           0x0A
#define FORM_FEED           0x0B
#define CARRIAGE_RETURN     0x0D
#define SUBSTITUTE          0x1A
#define DEL                 0x7F

// Macro definitions for UART config literals
//
// clang-format off

// Default configuration for UART peripheral
#define UART_CONFIG_DEFAULT         \
  (UART_Config) {                   \
    .OVER8  = UART_OVER8,           \
    .M      = UART_WORD8,           \
    .WAKE   = UART_WAKEUP_IDLE,     \
    .PCE    = false,                \
    .PS     = UART_PARITY_EVEN,     \
    .PEIE   = false,                \
    .TXEIE  = false,                \
    .TCIE   = false,                \
    .RXNEIE = false,                \
    .IDLEIE = false,                \
    .TE     = true,                 \
    .RE     = true,                 \
    .RWU    = UART_RECEIVER_ACTIVE  \
  }
// clang-format on

/**
 * @ingroup UART
 * @addtogroup UART_Interface Interface
 * @brief UART interface from which external peripherals inherit
 * @{
 */

/**
 * @brief   UART oversampling mode enum
 * @details Describes the number of bits to use in oversampling
 */
typedef enum {
  UART_OVER16, // Oversampling by 16
  UART_OVER8   // Oversampling by 8
} UART_OversampleMode;

/**
 * @brief   UART word length enum
 * @details Describes the number of bits to use in data word
 */
typedef enum {
  UART_WORD8, // Use 8 data bits
  UART_WORD9  // Use 9 data bits
} UART_WordLength;

/**
 * @brief   UART wake up method enum
 * @details Describes
 */
typedef enum {
  UART_WAKEUP_IDLE,   //
  UART_WAKEUP_ADDRESS //
} UART_Wake;

/**
 * @brief   UART parity selection enum
 * @details Describes odd/even parity type when parity is enabled
 */
typedef enum {
  UART_PARITY_EVEN, // Use even parity
  UART_PARITY_ODD   // Use odd parity
} UART_ParitySelect;

/**
 * @brief   UART receiver wake up enum
 * @details Describes if the UART is in mute mode or not
 */
typedef enum {
  UART_RECEIVER_ACTIVE, // Receiver is active
  UART_RECEIVER_MUTE    // Receiver is in mute mode
} UART_ReceiverWake;

typedef struct UART_Config {
  bool SBK                  : 1; //!<  | (default )
  UART_ReceiverWake RWU     : 1; //!<  | (default )
  bool RE                   : 1; //!<  | (default )
  bool TE                   : 1; //!<  | (default )
  bool IDLEIE               : 1; //!<  | (default )
  bool RXNEIE               : 1; //!<  | (default )
  bool TCIE                 : 1; //!<  | (default )
  bool TXEIE                : 1; //!<  | (default )
  bool PEIE                 : 1; //!<  | (default )
  UART_ParitySelect PS      : 1; //!<  | (default )
  bool PCE                  : 1; //!<  | (default )
  UART_Wake WAKE            : 1; //!<  | (default )
  UART_WordLength M         : 1; //!<  | (default )
  bool UE                   : 1; //!<  | (default )
  unsigned int              : 1; //!< RESERVED
  UART_OversampleMode OVER8 : 1; //!<  | (default )
} UART_Config;

/**
 * @brief Struct definition for \ref UART "UART interface"
 */
typedef struct UART {
  USART_TypeDef *interface;
  UART_Config config;
  uint32_t baud;
  UART_OversampleMode over8;
  void (*setBaud)(struct UART *, uint32_t);
  void (*send)(struct UART *, uint8_t);             //!< UART send method.   	             @see UART_send
  void (*sendBytes)(struct UART *, uint8_t *, int); //!< UART send multiple bytes method.  @see UART_sendBytes
  void (*print)(struct UART *, char *);             //!< UART print string method.  			 @see UART_print
  void (*println)(struct UART *, char *);           //!< UART print line method.  			 @see UART_print
  uint8_t (*receive)(struct UART *);                //!< UART receive method.              @see UART_receive
} UART_t;

UART_t UART_init(USART_TypeDef *interface, uint32_t baud, UART_Config *config);
void UART_updateConfig(UART_t *, UART_Config *);

void UART_setBaud(UART_t *, uint32_t);

void UART_send(UART_t *, uint8_t data);
void UART_sendBytes(UART_t *, uint8_t *data, int length);
void UART_print(UART_t *, char *data);
void UART_println(UART_t *, char *data);
uint8_t UART_receive(UART_t *);

/** @} */
#endif
