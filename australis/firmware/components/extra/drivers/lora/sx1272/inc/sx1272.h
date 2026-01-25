/**
 * @file sx1272.h
 * @brief SX1272 header
 */

// ALLOW FORMATTING
#ifndef _LORA_H
#define _LORA_H

#include "lora.h"

#include "spi.h"
#include "gpiopin.h"

#define SX1272_REG_END                       0x46

#define SX1272_REG_FIFO                      0x00
#define SX1272_REG_FIFO_ADDR_PTR             0x0D
#define SX1272_REG_FIFO_TX_BASE_ADDR         0x0E
#define SX1272_REG_FIFO_RX_BASE_ADDR         0x0F
#define SX1272_REG_FIFO_RX_CURR_ADDR         0x10

#define SX1272_REG_RX_BYTES                  0x13

#define SX1272_REG_DIO_MAPPING1              0x40
#define SX1272_DIO_MAPPING_DIO0_Pos          0x06
#define SX1272_DIO_MAPPING_DIO1_Pos          0x04
#define SX1272_DIO_MAPPING_DIO2_Pos          0x02
#define SX1272_DIO_MAPPING_DIO3_Pos          0x00
#define SX1272_DIO_MAPPING_DIO4_Pos          0x06
#define SX1272_DIO_MAPPING_DIO5_Pos          0x04

#define SX1272_LORA_DIO_RXDONE               0x00 << SX1272_DIO_MAPPING_DIO0_Pos
#define SX1272_LORA_DIO_TXDONE               0x01 << SX1272_DIO_MAPPING_DIO0_Pos

#define SX1272_REG_IRQ_FLAGS_MASK            0x11
#define SX1272_REG_IRQ_FLAGS                 0x12
#define SX1272_LORA_IRQ_RXDONE               0x40
#define SX1272_LORA_IRQ_TXDONE               0x08

#define SX1272_REG_OP_MODE                   0x01
#define SX1272_OP_MODE_LONG_RANGE            0x80
#define SX1272_OP_MODE_MODE_Pos              0x00

#define SX1272_REG_OCP                       0x0B
#define SX1272_OCP_ON                        0x20
#define SX1272_OCP_TRIM                      0x1F

#define SX1272_REG_FR_MSB                    0x06
#define SX1272_REG_FR_MIB                    0x07
#define SX1272_REG_FR_LSB                    0x08

#define SX1272_REG_MODEM_CONFIG1             0x1D
#define SX1272_MODEM_CONFIG1_CRC             0x02
#define SX1272_MODEM_CONFIG1_IMPLICIT_HEADER 0x04
#define SX1272_MODEM_CONFIG1_BW_Pos          0x06
#define SX1272_MODEM_CONFIG1_CR_Pos          0x03

#define SX1272_REG_MODEM_CONFIG2             0x1E
#define SX1272_MODEM_CONFIG2_AGC_AUTO_ON     0x04
#define SX1272_MODEM_CONFIG2_SF_Pos          0x04

#define SX1272_REG_PA_CONFIG                 0x09
#define SX1272_PA_SELECT                     0x80

#define SX1272_REG_LNA                       0x0C

#define SX1272_REG_PAYLOAD_LENGTH            0x22
#define SX1272_REG_MAX_PAYLOAD_LENGTH        0x23

#define SX1272_REG_PA_DAC                    0x5A

// Macro definitions for device config literals
//
// clang-format off

#define SX1272_CONFIG_DEFAULT         \
  (SX1272_Config) {                   \
    .freq             = 915.0f,       \
    .bw               = SX1272_BW500, \
    .sf               = SX1272_SF9,   \
    .cr               = SX1272_CR5,   \
    .implicitHeader   = false,        \
    .crc              = false,        \
    .paSelect         = true,         \
    .outputPower      = 0x1F,         \
    .ocp              = true,         \
    .ocpTrim          = 0x1B,         \
    .rxFifoBaseAddr   = 0x00,         \
    .txFifoBaseAddr   = 0x00,         \
    .maxPayloadLength = 0x20          \
  }
// clang-format on

/**
 * @ingroup Communication_LoRa
 * @addtogroup LoRa_SX1272
 * @brief SX1272 LoRa device driver.
 * @{
 */

/**
 * @brief   SX1272 bandwidth enum
 * @details Describes the occupied signal bandwidth
 */
typedef enum {
  SX1272_BW125, // 125kHz
  SX1272_BW250, // 250kHz
  SX1272_BW500, // 500kHz
} SX1272_Bandwidth;

/**
 * @brief   SX1272 coding rate enum
 * @details Describes the LoRa coding rate
 * TODO: describe what coding rate actually does
 */
typedef enum {
  SX1272_CR5 = 1, // 4/5
  SX1272_CR6,     // 4/6
  SX1272_CR7,     // 4/7
  SX1272_CR8,     // 4/8
} SX1272_CodingRate;

/**
 * @brief   SX1272 spreading factor enum
 * @details Describes the LoRa spreading factor
 * TODO: describe what spreading factor actually does
 */
typedef enum {
  SX1272_SF6 = 6,
  SX1272_SF7,
  SX1272_SF8,
  SX1272_SF9,
  SX1272_SF10,
  SX1272_SF11,
  SX1272_SF12,
} SX1272_SpreadingFactor;

/**
 * @brief   SX1272 operating mode enum
 * @details Describes the available operating modes on the transceiver
 */
typedef enum {
  SX1272_MODE_SLEEP,        // Low power mode. Only SPI and config registers available
  SX1272_MODE_STDBY,        // Standby mode. Chip is active, RF is disabled
  SX1272_MODE_FSTX,         // Frequency synthesis transmission mode
  SX1272_MODE_TX,           // Transmission mode
  SX1272_MODE_FSRX,         // Frequency synthesis receive mode
  SX1272_MODE_RXCONTINUOUS, // Continuous receive mode
  SX1272_MODE_RXSINGLE,     // Single receive mode
  SX1272_MODE_CAD           // Channel activity detection mode
} SX1272_Mode;

/**
 * @brief   SX1272 LoRa configuration struct
 * @details Describes the configuration of SX1272 LoRa parameters.
 */
typedef struct {
  float freq; //!< SX1272 LoRa carrier frequency

  // LoRa modem configuration
  // clang-format off
  SX1272_SpreadingFactor sf; //!< SX1272 LoRa modem spreading factor
  SX1272_Bandwidth bw;       //!< SX1272 LoRa modem bandwidth
  SX1272_CodingRate cr;      //!< SX1272 LoRa modem coding rate
  bool implicitHeader;       //!< SX1272 LoRa modem implicit header enable
  bool crc;                  //!< SX1272 LoRa modem CRC enable

  // LoRa power amplifier configuration
  bool paSelect;             //!< SX1272 LoRa modem power amp output select
  uint8_t outputPower;       //!< SX1272 LoRa modem power amp output power

  // LoRa over current protection configuration
  bool ocp;                  //!< SX1272 LoRa modem overcurrent protection enable
  uint8_t ocpTrim;           //!< SX1272 LoRa modem overcurrent protection trim

  // FIFO configuration
  uint8_t rxFifoBaseAddr;    //!< Base address for RX fifo
  uint8_t txFifoBaseAddr;    //!< Base address for TX fifo
  uint8_t maxPayloadLength;  //!< Maximum allowed length of payload
  // clang-format on
} SX1272_Config;

/**
 * @brief Struct definition for SX1272.
 * Provides the interface for API consumers to interact with the SX1272 LoRa transceiver.
 */
typedef struct SX1272 {
  LoRa_t base;                                            //!< Base LoRa API.
  SX1272_Config config;                                   //!< Configuration parameters for the SX1272 driver
  SPI_t *spi;                                             //!< Parent SPI interface.
  GPIOpin_t cs;                                           //!< Chip select GPIO.
  SX1272_Mode currentMode;                                //!< Current operating mode.
  void (*standby)(struct SX1272 *);                       //!< SX1272 standby method.              @see SX1272_standby
  bool (*updateConfig)(struct SX1272 *, SX1272_Config *); //!< SX1272 configuration update method. @see SX1272_updateConfig
} SX1272_t;

bool SX1272_init(SX1272_t *lora, SPI_t *spi, GPIOpin_t cs, SX1272_Config *config);
bool SX1272_updateConfig(SX1272_t *lora, SX1272_Config *config);

void SX1272_transmit(LoRa_t *lora, uint8_t *data, uint8_t length);
void SX1272_startReceive(LoRa_t *lora);
uint8_t SX1272_readReceive(LoRa_t *lora, uint8_t *data, uint8_t byffSize);

void SX1272_standby(SX1272_t *);
void SX1272_clearIRQ(LoRa_t *, uint8_t);

void _SX1272_setMode(SX1272_t *, SX1272_Mode);

uint8_t SX1272_readRegister(SX1272_t *, uint8_t);

#endif
