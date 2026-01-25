/**
 * @author Matt Ricci
 * @addtogroup API API Reference
 * @{
 * @addtogroup SPI
 * @}
 */

#ifndef SPI_H
#define SPI_H

#include "stm32f439xx.h"
#include "stdint.h"
#include "stdbool.h"

// Macro definitions for pin config literals
//
// clang-format off

#define SPI_CONFIG_DEFAULT     \
  SPI_CONFIG_FULLDUPLEX_MASTER

// Default configuration for full-duplex master mode
#define SPI_CONFIG_FULLDUPLEX_MASTER \
  (SPI_Config) {                     \
    .CPHA     = SPI_CPHA_SECOND,     \
    .CPOL     = SPI_CPOL1,           \
    .MSTR     = SPI_MASTER,          \
    .BR       = SPI_BR_PCLK8,        \
    .LSBFIRST = SPI_MSB_FIRST,       \
    .SSI      = true,                \
    .SSM      = true,                \
    .RXONLY   = false,               \
    .DFF      = SPI_DFF8,            \
    .CRCNEXT  = false,               \
    .CRCEN    = false,               \
    .BIDIOE   = false,               \
    .BIDIMODE = false                \
  }
// clang-format on

/**
 * @ingroup SPI
 * @addtogroup SPI_Interface Interface
 * @brief SPI interface for communicating with exeternal devices.
 * @{
 */

/**
 * @brief   SPI clock phase enum
 * @details Describes when to begin capturing data
 */
typedef enum {
  SPI_CPHA_FIRST, // Begin data capture on first clock transition
  SPI_CPHA_SECOND // Begin data capture on second clock transition
} SPI_Phase;

/**
 * @brief   SPI clock polarity enum
 * @details Describes logic level of clock when idle
 */
typedef enum {
  SPI_CPOL0, // Clock idles low
  SPI_CPOL1  // Clock idles high
} SPI_Polarity;

/**
 * @brief   SPI master selection enum
 * @details Describes operational mode of SPI
 */
typedef enum {
  SPI_SLAVE, // Slave configuration
  SPI_MASTER // Master configuration
} SPI_MasterSelect;

/**
 * @brief   SPI baud rate enum
 * @details Describes clock division for SPI peripheral
 */
typedef enum {
  SPI_BR_PCLK2,   // Pclk/2
  SPI_BR_PCLK4,   // Pclk/4
  SPI_BR_PCLK8,   // Pclk/8
  SPI_BR_PCLK16,  // Pclk/16
  SPI_BR_PCLK32,  // Pclk/32
  SPI_BR_PCLK64,  // Pclk/64
  SPI_BR_PCLK128, // Pclk/128
  SPI_BR_PCLK256, // Pclk/256
} SPI_BaudRate;

/**
 * @brief   SPI data format enum
 * @details Describes the format of data sent on the SPI data bus.
 */
typedef enum {
  SPI_DFF8, // 8-bit data frame
  SPI_DFF16 // 16-bit data frame
} SPI_DataFormat;

/**
 * @brief   SPI frame format enum
 * @details Describes the order in which to transmit bits per frame
 */
typedef enum {
  SPI_MSB_FIRST, // Transmit MSB first
  SPI_LSB_FIRST  // Transmit LSB first
} SPI_FrameFormat;

/**
 * @brief   SPI CR1 configuration struct
 * @details Describes the configuration of control register 1.
 */
typedef struct {
  SPI_Phase CPHA           : 1; //!< SPI clock phase               | (default SPI_CPHA_SECOND)
  SPI_Polarity CPOL        : 1; //!< SPI clock polarity            | (default SPI_CPOL1)
  SPI_MasterSelect MSTR    : 1; //!< SPI master select             | (default SPI_MASTER)
  SPI_BaudRate BR          : 3; //!< SPI clock division            | (default SPI_BR_PCLK8)
  bool SPE                 : 1; //!< SPI enable                    | (default false)
  SPI_FrameFormat LSBFIRST : 1; //!< SPI frame format              | (default SPI_MSB_FIRST)
  bool SSI                 : 1; //!< SPI internal slave select     | (default true)
  bool SSM                 : 1; //!< SPI software slave management | (default true )
  bool RXONLY              : 1; //!< SPI receive only              | (default false)
  SPI_DataFormat DFF       : 1; //!< SPI data format               | (default SPI_DFF8)
  bool CRCNEXT             : 1; //!< SPI CRC transfer next         | (default false)
  bool CRCEN               : 1; //!< SPI CRC enable                | (default false)
  bool BIDIOE              : 1; //!< SPI bidirectional output      | (default false)
  bool BIDIMODE            : 1; //!< SPI bidirectional mode        | (default true)
} SPI_Config;

/**
 * @brief Struct definition for \ref SPI "SPI interface".
 * Provides the interface for API consumers to interact with the SPI peripheral.
 */
typedef struct SPI {
  SPI_TypeDef *interface;                             //!< Pointer to SPI interface struct.
  SPI_Config config;                                  //!< Configuration parameters for the SPI peripheral.
  void (*send)(struct SPI *, uint16_t);               //!< SPI send method.                 @see SPI_send
  void (*receive)(struct SPI *, volatile uint16_t *); //!< SPI receive method.              @see SPI_receive
  uint16_t (*transmit)(struct SPI *, uint16_t);       //!< SPI transmit method.             @see SPI_transmit
  void (*updateConfig)(struct SPI *, SPI_Config *);   //!< SPI configuration update method. @see SPI_updateConfig
} SPI_t;

SPI_t SPI_init(SPI_TypeDef *, SPI_Config *);
void SPI_send(SPI_t *, uint16_t);
void SPI_receive(SPI_t *, volatile uint16_t *);
uint16_t SPI_transmit(SPI_t *, uint16_t);
void SPI_updateConfig(SPI_t *, SPI_Config *);

/** @} */
#endif
