/**************************************************************************************************
 * @file        rfm95.c                                                                          *
 * @author      Matt Ricci                                                                        *
 * @addtogroup  LoRa                                                                              *
 * @brief       Brief description of the file's purpose.                                          *
 *                                                                                                *
 * @todo Implement adjustable packet size                                                         *
 * @{                                                                                             *
 **************************************************************************************************/

#include "gpiopin.h"
#include "lora.h"
#include "spi.h"
#include "rfm95.h"

#include "stddef.h"
#include "stdint.h"

static void _RFM95_init(RFM95_t *, RFM95_Config *);
static void RFM95_writeRegister(RFM95_t *, uint8_t, uint8_t);

/* ============================================================================================== */
/**
 * @brief  Initializes the LoRa module with specified configuration parameters.
 *
 * @param  *lora   Pointer to LoRa struct to be initialised.
 * @param  *spi    Pointer to SPI peripheral struct.
 * @param  *config
 *
 * @return Ininitialised RFM95 device struct.
 **
 * ============================================================================================== */
bool RFM95_init(RFM95_t *lora, SPI_t *spi, GPIOpin_t cs, RFM95_Config *config) {
  lora->spi               = spi;
  lora->cs                = cs;
  lora->standby           = RFM95_standby;
  lora->updateConfig      = RFM95_updateConfig;
  lora->base.transmit     = RFM95_transmit;
  lora->base.startReceive = RFM95_startReceive;
  lora->base.readReceive  = RFM95_readReceive;
  lora->base.clearIRQ     = RFM95_clearIRQ;

  return RFM95_updateConfig(lora, config);
}

// ALLOW FORMATTING
#ifndef __DOXYGEN__

/**************************************** PRIVATE METHODS *****************************************/

/* ============================================================================================== */
/**
 * @brief   Private initialiser for RFM95 configuration registers.
 *
 * @param   config
 *
 * @return  @c NULL.
 **
 * ============================================================================================== */
static void _RFM95_init(RFM95_t *lora, RFM95_Config *config) {
  // Set mode to sleep
  _RFM95_setMode(lora, RFM95_MODE_SLEEP);

  /* clang-format off */
  uint8_t opMode = RFM95_readRegister(lora, RFM95_REG_OP_MODE); 
  RFM95_writeRegister(lora, RFM95_REG_OP_MODE, opMode | RFM95_OP_MODE_LONG_RANGE); 

  _RFM95_setMode(lora, RFM95_MODE_STDBY);

  uint32_t frf = (config->freq * (0x01 << 19)) / 32.0f;
  RFM95_writeRegister(lora, RFM95_REG_FR_LSB, frf >> 0);
  RFM95_writeRegister(lora, RFM95_REG_FR_MIB, frf >> 8);
  RFM95_writeRegister(lora, RFM95_REG_FR_MSB, frf >> 16);

  
  // Set bandwidth, coding rate, toggle implicit header mode
  RFM95_writeRegister(lora, RFM95_REG_MODEM_CONFIG1, 
    config->bw << RFM95_MODEM_CONFIG1_BW_Pos                           // Set bandwidth
  | config->cr << RFM95_MODEM_CONFIG1_CR_Pos                           // Set coding rate
  | (config->implicitHeader ? RFM95_MODEM_CONFIG1_IMPLICIT_HEADER : 0) // Set header mode
  );

  // Set spreading factor and CRC
  RFM95_writeRegister(lora, RFM95_REG_MODEM_CONFIG2, 
    (config->sf << RFM95_MODEM_CONFIG2_SF_Pos)
  | (config->crc ? RFM95_MODEM_CONFIG2_CRC_ON : 0)
  );

  // Set maximum payload length
  RFM95_writeRegister(lora, RFM95_REG_MAX_PAYLOAD_LENGTH, config->maxPayloadLength);

  // Set FIFO base addresses
  RFM95_writeRegister(lora, RFM95_REG_FIFO_TX_BASE_ADDR, config->txFifoBaseAddr); 
  RFM95_writeRegister(lora, RFM95_REG_FIFO_RX_BASE_ADDR, config->rxFifoBaseAddr); 

  // Set over current protection configuration
  RFM95_writeRegister(lora, RFM95_REG_OCP,
    (config->ocp ? RFM95_OCP_ON : 0) // Enable/disable over current protection
   | config->ocpTrim                  // Set overcurrent protection trim
  );

  // Set power amplifier configuration
  RFM95_writeRegister(lora, RFM95_REG_PA_CONFIG,
    (config->paSelect ? RFM95_PA_SELECT : 0) // Select power amplifier output pin
  | config->outputPower                       // Set power amplifier output power
  );
  /* clang-format on */

  // Enable +20dBm on PA_BOOST if it's on.
  if (config->paSelect) {
    RFM95_writeRegister(lora, RFM95_REG_PA_DAC, 0x07);
  }
  
  // Set mode to standby
  //_RFM95_setMode(lora, RFM95_MODE_STDBY);
}

/* ============================================================================================== */
/**
 * @brief  Sets the operational mode of the LoRa module.
 *
 * @param  *lora        Pointer to LoRa struct.
 * @param  mode         Desired operational mode to be set.
 * @return @c NULL.
 **
 * ============================================================================================== */
void _RFM95_setMode(RFM95_t *lora, RFM95_Mode mode) {
  uint8_t regOpMode  = RFM95_readRegister(lora, RFM95_REG_OP_MODE);
  regOpMode         &= ~0x07; // Mask to mode bits
  regOpMode         |= mode;  // Set mode
  lora->currentMode  = mode;  // Set driver mode

  switch (mode) {
    case RFM95_MODE_TX:
      lora->base.currentMode = LORA_MODE_TX;
      break;
    case RFM95_MODE_RXCONTINUOUS:
      lora->base.currentMode = LORA_MODE_RX;
      break;
    default:
      lora->base.currentMode = LORA_MODE_OTHER;
      break;
  }

  // Update device mode
  RFM95_writeRegister(lora, RFM95_REG_OP_MODE, regOpMode);
}

#endif

/***************************************** PUBLIC METHODS *****************************************/

/* ============================================================================================== */
/**
 * @brief  Sets the operational mode of the LoRa module to standby.
 *
 * @param  *lora Pointer to LoRa struct.
 *
 * @return @c NULL.
 **
 * ============================================================================================== */
void RFM95_standby(RFM95_t *lora) {
  _RFM95_setMode(lora, RFM95_MODE_STDBY);
}

/* ============================================================================================== */
/**
 * @brief Transmits data using the RFM95.
 *
 * @param lora         Pointer to RFM95 struct.
 * @param pointerdata  Pointer to the data to be transmitted.
 **
 * ============================================================================================== */
void RFM95_transmit(LoRa_t *lora, uint8_t *pointerdata, uint8_t length) {
  RFM95_t *driver = (RFM95_t *)lora;

  // Set device to standby
  _RFM95_setMode(driver, RFM95_MODE_STDBY);

  // Set payload length
  RFM95_writeRegister(driver, RFM95_REG_PAYLOAD_LENGTH, length);

  // TODO: There is no DIO IRQ flags...
  // TODO: add in proper read-mask-write operation for setting DIO mapping
  //
  // Set DIO interrupt pin to TxDone
  // RFM95_writeRegister(driver, RFM95_REG_DIO_MAPPING1, RFM95_LORA_DIO_TXDONE);

  // Since the device will only ever be transmitting or receiving at any given time
  // and each packet should be handled immediately by the implementation (no waiting
  // on buffering), we don't need to be concerned about the buffer being overwritten.
  //
  // ...for now.

  // TODO:
  // Think of a more elegant solution for applications that might use this
  // driver that want buffered data
  //

  // Clear IRQ flags and set FIFO address pointer.
  RFM95_writeRegister(driver, RFM95_REG_IRQ_FLAGS, RFM95_LORA_IRQ_TXDONE); // clears the IRQ flag
  RFM95_writeRegister(driver, RFM95_REG_FIFO_ADDR_PTR, 0x00);              // set pointer adddress to start
  // Load data into transmit FIFO
  for (int i = 0; i < length; i++) {
    RFM95_writeRegister(driver, RFM95_REG_FIFO, pointerdata[i]);
  }

  // Update the current operating mode
  _RFM95_setMode(driver, RFM95_MODE_TX); // Start transmitting
}

/* ============================================================================================== */
/**
 * @brief Begins continuous receive on the RFM95.
 *
 * @param lora Pointer to RFM95 struct.
 **
 * ============================================================================================== */
void RFM95_startReceive(LoRa_t *lora) {
  RFM95_t *driver = (RFM95_t *)lora;

  // Set device to standby
  _RFM95_setMode(driver, RFM95_MODE_STDBY);

  // TODO: add in proper read-mask-write operation for setting DIO mapping
  //
  // Set DIO interrupt pin to RxDone
  // RFM95_writeRegister(driver, RFM95_REG_DIO_MAPPING1, RFM95_LORA_DIO_RXDONE);

  // Since the device will only ever be transmitting or receiving at any given time
  // and each packet should be handled immediately by the implementation (no waiting
  // on buffering), we don't need to be concerned about the buffer being overwritten.
  //
  // ...for now.

  // TODO:
  // Think of a more elegant solution for applications that might use this
  // driver that want buffered data
  //
  // Clear IRQ flags and set FIFO address pointer.
  // RFM95_writeRegister(driver, RFM95_REG_IRQ_FLAGS, RFM95_LORA_IRQ_RXDONE); // Clear the IRQ flag
  RFM95_writeRegister(driver, RFM95_REG_FIFO_ADDR_PTR, 0x00); // Set pointer adddress to start

  // Update the current operating mode
  _RFM95_setMode(driver, RFM95_MODE_RXCONTINUOUS); // Start receiving
}

/* ============================================================================================== */
/**
 * @brief  Reads contents of received packet to local buffer from the RFM95.
 *
 * @param  lora     Pointer to RFM95 struct.
 * @param  buffer   Pointer to the buffer to store received data.
 * @param  buffSize Integer representing the size of the buffer to to fill.
 *
 * @return Boolean value indicating if a packet was successfully received and
 *         returned in buffer.
 **
 * ============================================================================================== */
uint8_t RFM95_readReceive(LoRa_t *lora, uint8_t *buffer, uint8_t buffSize) {
  RFM95_t *driver = (RFM95_t *)lora;

  // TODO: Error handling for IRQ flags
  //
  // Currently the readReceive() method clears the RFM95 RxDone IRQ flag before
  // starting the read. This is fine for cases where the user code carefully
  // manages the DIO interrupts, however ideally the method should check for
  // errors in the IRQ register and appropriately discard received packets.

  // Clear the IRQ flag
  RFM95_writeRegister(driver, RFM95_REG_IRQ_FLAGS, RFM95_LORA_IRQ_RXDONE);

  // Read address and packet width information of received data
  uint8_t bytesReceived = RFM95_readRegister(driver, RFM95_REG_RX_BYTES);          // Number of bytes received
  uint8_t rxCurrentAddr = RFM95_readRegister(driver, RFM95_REG_FIFO_RX_CURR_ADDR); // Address of last packet

  // Return error if buffer is smaller than the received data
  if (bytesReceived > buffSize) {
    return false;
  }

  // Otherwise, set the address pointer and read each byte into buffer
  RFM95_writeRegister(driver, RFM95_REG_FIFO_ADDR_PTR, rxCurrentAddr);
  for (int i = 0; i < bytesReceived; i++) {
    buffer[i] = RFM95_readRegister(driver, RFM95_REG_FIFO);
  }

  return bytesReceived;
}

/* ============================================================================================== */
/**
 * @brief  Sets the value of RegIrqFlags in the RFM95 to the provided argument value.
 *         Writing a 1 to a bit in the register will clear the associated flag.
 *
 * @param  lora  Pointer to RFM95 struct.
 * @param  flags 8-bit value representing flag bits to be set.
 *
 * @return @c NULL
 **
 * ============================================================================================== */
void RFM95_clearIRQ(LoRa_t *lora, uint8_t flags) {
  RFM95_writeRegister((RFM95_t *)lora, RFM95_REG_IRQ_FLAGS, flags);
}

/* ============================================================================================== */
/**
 * @brief   Update RFM95 configuration
 * @details
 *
 * @param   spi Pointer to SPI_t struct.
 *
 * @return  @c NULL.
 **
 * ============================================================================================== */
bool RFM95_updateConfig(RFM95_t *lora, RFM95_Config *config) {
  // Initialise config with default values if passed NULL.
  if (config == NULL) {
    config = &RFM95_CONFIG_DEFAULT;
  }

  // Validate max values
  if (config->outputPower > 0x0F || config->ocpTrim > 0x1F) {
    return false;
  }

  // Update peripheral with new config
  lora->config = *config;

  // Initialise SPI registers and enable peripheral
  _RFM95_init(lora, config);

  return true;
}

/*************************************** INTERFACE METHODS ****************************************/

// NOTE: This function is left exposed to public imports for debug purposes
uint8_t RFM95_readRegister(RFM95_t *lora, uint8_t address) {
  uint8_t response = 0;
  SPI_t *spi       = lora->spi;
  GPIOpin_t cs     = lora->cs;

  // Pull CS low
  cs.reset(&cs);

  // Send write data and address
  uint8_t payload = address & 0x7F;              // Load payload with address and read command
  response        = spi->transmit(spi, payload); // Transmit payload
  response        = spi->transmit(spi, 0xFF);    // Transmit dummy data and reasd response

  // Set CS high
  cs.set(&cs);

  return response;
}

void RFM95_writeRegister(RFM95_t *lora, uint8_t address, uint8_t data) {
  SPI_t *spi   = lora->spi;
  GPIOpin_t cs = lora->cs;

  // Pull CS low
  cs.reset(&cs);

  // Send write data and address
  uint8_t payload = address | 0x80; // Load payload with address and write command
  spi->transmit(spi, payload);      // Transmit payload
  spi->transmit(spi, data);         // Transmit write data

  // Set CS high
  cs.set(&cs);
}

/** @} */
