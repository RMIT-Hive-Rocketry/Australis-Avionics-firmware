/**
 * @file rfm95.h
 * @brief RFM95 header
 */

// ALLOW FORMATTING
#ifndef _LORA_H
#define _LORA_H

#include "lora.h"

#include "stdint.h"

#include "spi.h"
#include "gpiopin.h"

#define RFM95_REG_END                       0x46

#define RFM95_REG_FIFO                      0x00
#define RFM95_REG_FIFO_ADDR_PTR             0x0D
#define RFM95_REG_FIFO_TX_BASE_ADDR         0x0E
#define RFM95_REG_FIFO_RX_BASE_ADDR         0x0F
#define RFM95_REG_FIFO_RX_CURR_ADDR         0x10

#define RFM95_REG_RX_BYTES                  0x13

#define RFM95_REG_DIO_MAPPING1              0x40
#define RFM95_DIO_MAPPING_DIO0_Pos          0x06
#define RFM95_DIO_MAPPING_DIO1_Pos          0x04
#define RFM95_DIO_MAPPING_DIO2_Pos          0x02
#define RFM95_DIO_MAPPING_DIO3_Pos          0x00
#define RFM95_DIO_MAPPING_DIO4_Pos          0x06
#define RFM95_DIO_MAPPING_DIO5_Pos          0x04

#define RFM95_LORA_DIO_RXDONE               0x00 << RFM95_DIO_MAPPING_DIO0_Pos
#define RFM95_LORA_DIO_TXDONE               0x01 << RFM95_DIO_MAPPING_DIO0_Pos

#define RFM95_REG_IRQ_FLAGS_MASK            0x11
#define RFM95_REG_IRQ_FLAGS                 0x12
#define RFM95_LORA_IRQ_RXDONE               0x40
#define RFM95_LORA_IRQ_TXDONE               0x08

#define RFM95_REG_OP_MODE                   0x01
#define RFM95_OP_MODE_LONG_RANGE            0x80
#define RFM95_OP_MODE_MODE_Pos              0x00

#define RFM95_REG_OCP                       0x0B
#define RFM95_OCP_ON                        0x20
#define RFM95_OCP_TRIM                      0x1F

#define RFM95_REG_FR_MSB                    0x06
#define RFM95_REG_FR_MIB                    0x07
#define RFM95_REG_FR_LSB                    0x08

#define RFM95_REG_MODEM_CONFIG1             0x1D
#define RFM95_MODEM_CONFIG1_IMPLICIT_HEADER 0x01
#define RFM95_MODEM_CONFIG1_BW_Pos          0x04
#define RFM95_MODEM_CONFIG1_CR_Pos          0x01

#define RFM95_REG_MODEM_CONFIG2             0x1E
#define RFM95_MODEM_CONFIG2_CRC_ON          0x04
#define RFM95_MODEM_CONFIG2_AGC_AUTO_ON     0x04
#define RFM95_MODEM_CONFIG2_SF_Pos          0x04

#define RFM95_REG_PA_CONFIG                 0x09
#define RFM95_PA_SELECT                     0x80

#define RFM95_REG_LNA                       0x0C

#define RFM95_REG_PAYLOAD_LENGTH            0x22
#define RFM95_REG_MAX_PAYLOAD_LENGTH        0x23

#define RFM95_REG_PA_DAC                    0x4D

// Macro definitions for device config literals
//
// clang-format off

#define RFM95_CONFIG_DEFAULT         \
  (RFM95_Config) {                   \
    .freq             = 915.0f,     \
    .bw               = RFM95_BW500, \
    .sf               = RFM95_SF9,   \
    .cr               = RFM95_CR5,   \
    .implicitHeader   = false,      \
    .crc              = false,      \
    .paSelect         = true,       \
    .outputPower      = 0x0F,       \
    .ocp              = true,       \
    .ocpTrim          = 0x1B,       \
    .rxFifoBaseAddr   = 0x00,       \
    .txFifoBaseAddr   = 0x00,       \
    .maxPayloadLength = 0x20        \
  }
// clang-format on

/**
 * @ingroup Communication_LoRa
 * @addtogroup LoRa_RFM95
 * @brief RFM95 LoRa device driver.
 * @{
 */

/**
 * @brief   RFM95 bandwidth enum
 * @details Describes the occupied signal bandwidth
 */
typedef enum {
  RFM95_BW125, // 125kHz
  RFM95_BW250, // 250kHz
  RFM95_BW500, // 500kHz
} RFM95_Bandwidth;

/**
 * @brief   RFM95 coding rate enum
 * @details Describes the LoRa coding rate
 * TODO: describe what coding rate actually does
 */
typedef enum {
  RFM95_CR5 = 1, // 4/5
  RFM95_CR6,     // 4/6
  RFM95_CR7,     // 4/7
  RFM95_CR8,     // 4/8
} RFM95_CodingRate;

/**
 * @brief   RFM95 spreading factor enum
 * @details Describes the LoRa spreading factor
 * TODO: describe what spreading factor actually does
 */
typedef enum {
  RFM95_SF6 = 6,
  RFM95_SF7,
  RFM95_SF8,
  RFM95_SF9,
  RFM95_SF10,
  RFM95_SF11,
  RFM95_SF12,
} RFM95_SpreadingFactor;

/**
 * @brief   RFM95 operating mode enum
 * @details Describes the available operating modes on the transceiver
 */
typedef enum {
  RFM95_MODE_SLEEP,        // Low power mode. Only SPI and config registers available
  RFM95_MODE_STDBY,        // Standby mode. Chip is active, RF is disabled
  RFM95_MODE_FSTX,         // Frequency synthesis transmission mode
  RFM95_MODE_TX,           // Transmission mode
  RFM95_MODE_FSRX,         // Frequency synthesis receive mode
  RFM95_MODE_RXCONTINUOUS, // Continuous receive mode
  RFM95_MODE_RXSINGLE,     // Single receive mode
  RFM95_MODE_CAD           // Channel activity detection mode
} RFM95_Mode;

/**
 * @brief   RFM95 LoRa configuration struct
 * @details Describes the configuration of RFM95 LoRa parameters.
 */
typedef struct {
  float freq; //!< RFM95 LoRa carrier frequency

  // LoRa modem configuration
  // clang-format off
  RFM95_SpreadingFactor sf;   //!< RFM95 LoRa modem spreading factor
  RFM95_Bandwidth bw;         //!< RFM95 LoRa modem bandwidth
  RFM95_CodingRate cr;        //!< RFM95 LoRa modem coding rate
  bool implicitHeader;       //!< RFM95 LoRa modem implicit header enable
  bool crc;                  //!< RFM95 LoRa modem CRC enable

  // LoRa power amplifier configuration
  bool paSelect;             //!< RFM95 LoRa modem power amp output select
  uint8_t outputPower;       //!< RFM95 LoRa modem power amp output power

  // LoRa over current protection configuration
  bool ocp;                  //!< RFM95 LoRa modem overcurrent protection enable
  uint8_t ocpTrim;           //!< RFM95 LoRa modem overcurrent protection trim

  // FIFO configuration
  uint8_t rxFifoBaseAddr;    //!< Base address for RX fifo
  uint8_t txFifoBaseAddr;    //!< Base address for TX fifo
  uint8_t maxPayloadLength;  //!< Maximum allowed length of payload
  // clang-format on
} RFM95_Config;

/**
 * @brief Struct definition for RFM95.
 * Provides the interface for API consumers to interact with the RFM95 LoRa transceiver.
 */
typedef struct RFM95 {
  LoRa_t base;                                          //!< Base LoRa API.
  RFM95_Config config;                                  //!< Configuration parameters for the RFM95 driver
  SPI_t *spi;                                           //!< Parent SPI interface.
  GPIOpin_t cs;                                         //!< Chip select GPIO.
  RFM95_Mode currentMode;                               //!< Current operating mode.
  void (*standby)(struct RFM95 *);                      //!< RFM95 standby method.              @see RFM95_standby
  bool (*updateConfig)(struct RFM95 *, RFM95_Config *); //!< RFM95 configuration update method. @see RFM95_updateConfig
} RFM95_t;

bool RFM95_init(RFM95_t *lora, SPI_t *spi, GPIOpin_t cs, RFM95_Config *config);
bool RFM95_updateConfig(RFM95_t *lora, RFM95_Config *config);

void RFM95_transmit(LoRa_t *lora, uint8_t *data, uint8_t length);
void RFM95_startReceive(LoRa_t *lora);
uint8_t RFM95_readReceive(LoRa_t *lora, uint8_t *data, uint8_t byffSize);

void RFM95_standby(RFM95_t *);
void RFM95_clearIRQ(LoRa_t *, uint8_t);

void _RFM95_setMode(RFM95_t *, RFM95_Mode);

uint8_t RFM95_readRegister(RFM95_t *, uint8_t);

#endif
