/**************************************************************************************************
 * @file        sx1272.c                                                                          *
 * @author      Matt Ricci                                                                        *
 * @addtogroup  LoRa                                                                              *
 * @brief       Brief description of the file's purpose.                                          *
 *                                                                                                *
 * @todo Implement adjustable packet size                                                         *
 * @{                                                                                             *
 **************************************************************************************************/

#include "sx1272.h"

#include "stddef.h"
#include "math.h"

static void _SX1272_init(SX1272_t *, SX1272_Config *);
static void SX1272_writeRegister(SX1272_t *, uint8_t, uint8_t);

/* ============================================================================================== */
/**
 * @brief  Initializes the LoRa module with specified configuration parameters.
 *
 * @param  *lora   Pointer to LoRa struct to be initialised.
 * @param  *spi    Pointer to SPI peripheral struct.
 * @param  *config
 *
 * @return Ininitialised SX1272 device struct.
 **
 * ============================================================================================== */
bool SX1272_init(SX1272_t *lora, SPI_t *spi, GPIOpin_t cs, SX1272_Config *config) {
  lora->spi               = spi;
  lora->cs                = cs;
  lora->standby           = SX1272_standby;
  lora->updateConfig      = SX1272_updateConfig;
  lora->base.transmit     = SX1272_transmit;
  lora->base.startReceive = SX1272_startReceive;
  lora->base.readReceive  = SX1272_readReceive;
  lora->base.clearIRQ     = SX1272_clearIRQ;

  return SX1272_updateConfig(lora, config);
}

// ALLOW FORMATTING
#ifndef __DOXYGEN__

/**************************************** PRIVATE METHODS *****************************************/

/* ============================================================================================== */
/**
 * @brief   Private initialiser for SX1272 configuration registers.
 *
 * @param   config
 *
 * @return  @c NULL.
 **
 * ============================================================================================== */
static void _SX1272_init(SX1272_t *lora, SX1272_Config *config) {
  // Set mode to sleep
  _SX1272_setMode(lora, SX1272_MODE_SLEEP);

  /* clang-format off */
  uint8_t opMode = SX1272_readRegister(lora, SX1272_REG_OP_MODE); 
  SX1272_writeRegister(lora, SX1272_REG_OP_MODE, opMode | SX1272_OP_MODE_LONG_RANGE); 

  _SX1272_setMode(lora, SX1272_MODE_STDBY);

  uint32_t frf = (config->freq * (0x01 << 19)) / 32.0f;
  SX1272_writeRegister(lora, SX1272_REG_FR_LSB, frf >> 0);
  SX1272_writeRegister(lora, SX1272_REG_FR_MIB, frf >> 8);
  SX1272_writeRegister(lora, SX1272_REG_FR_MSB, frf >> 16);

  
  // Set bandwidth, coding rate, toggle implicit header mode, crc enable
  SX1272_writeRegister(lora, SX1272_REG_MODEM_CONFIG1, 
    config->bw << SX1272_MODEM_CONFIG1_BW_Pos                           // Set bandwidth
  | config->cr << SX1272_MODEM_CONFIG1_CR_Pos                           // Set coding rate
  | (config->implicitHeader ? SX1272_MODEM_CONFIG1_IMPLICIT_HEADER : 0) // Set header mode
  | (config->crc            ? SX1272_MODEM_CONFIG1_CRC             : 0) // Set CRC
  );

  // Set spreading factor
  SX1272_writeRegister(lora, SX1272_REG_MODEM_CONFIG2, 
    (config->sf << SX1272_MODEM_CONFIG2_SF_Pos) | SX1272_MODEM_CONFIG2_AGC_AUTO_ON
  );

  // Set maximum payload length
  SX1272_writeRegister(lora, SX1272_REG_MAX_PAYLOAD_LENGTH, config->maxPayloadLength);

  // Set FIFO base addresses
  SX1272_writeRegister(lora, SX1272_REG_FIFO_TX_BASE_ADDR, config->txFifoBaseAddr); 
  SX1272_writeRegister(lora, SX1272_REG_FIFO_RX_BASE_ADDR, config->rxFifoBaseAddr); 

  // Set over current protection configuration
  SX1272_writeRegister(lora, SX1272_REG_OCP,
    (config->ocp ? SX1272_OCP_ON : 0) // Enable/disable over current protection
   | config->ocpTrim                  // Set overcurrent protection trim
  );

  // Set power amplifier configuration
  SX1272_writeRegister(lora, SX1272_REG_PA_CONFIG,
    (config->paSelect ? SX1272_PA_SELECT : 0) // Select power amplifier output pin
  | config->outputPower                       // Set power amplifier output power
  );
  /* clang-format on */

  SX1272_writeRegister(lora, SX1272_REG_PA_DAC, 0x87);

  uint8_t RegPll  = SX1272_readRegister(lora, 0x5C);
  RegPll         &= 0xC0;
  RegPll         |= 0x40;
  SX1272_writeRegister(lora, 0x5C, RegPll);

  uint8_t RegTcxo  = SX1272_readRegister(lora, 0x58);
  RegTcxo         |= 0x10;
  SX1272_writeRegister(lora, 0x58, RegTcxo);

  SX1272_writeRegister(lora, 0x0A, 0x09);

  // Set mode to standby
  //_SX1272_setMode(lora, SX1272_MODE_STDBY);
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
void _SX1272_setMode(SX1272_t *lora, SX1272_Mode mode) {
  uint8_t regOpMode  = SX1272_readRegister(lora, SX1272_REG_OP_MODE);
  regOpMode         &= ~0x07; // Mask to mode bits
  regOpMode         |= mode;  // Set mode
  lora->currentMode  = mode;  // Set driver mode

  switch (mode) {
  case SX1272_MODE_TX:
    lora->base.currentMode = LORA_MODE_TX;
    break;
  case SX1272_MODE_RXCONTINUOUS:
    lora->base.currentMode = LORA_MODE_RX;
    break;
  default:
    lora->base.currentMode = LORA_MODE_OTHER;
    break;
  }

  // Update device mode
  SX1272_writeRegister(lora, SX1272_REG_OP_MODE, regOpMode);
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
void SX1272_standby(SX1272_t *lora) {
  _SX1272_setMode(lora, SX1272_MODE_STDBY);
}

/* ============================================================================================== */
/**
 * @brief Transmits data using the SX1272.
 *
 * @param lora         Pointer to SX1272 struct.
 * @param pointerdata  Pointer to the data to be transmitted.
 **
 * ============================================================================================== */
void SX1272_transmit(LoRa_t *lora, uint8_t *pointerdata, uint8_t length) {
  SX1272_t *driver = (SX1272_t *)lora;

  // Set device to standby
  _SX1272_setMode(driver, SX1272_MODE_STDBY);

  // Set payload length
  SX1272_writeRegister(driver, SX1272_REG_PAYLOAD_LENGTH, length);

  // TODO: add in proper read-mask-write operation for setting DIO mapping
  //
  // Set DIO interrupt pin to TxDone
  SX1272_writeRegister(driver, SX1272_REG_DIO_MAPPING1, SX1272_LORA_DIO_TXDONE);

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

  SX1272_writeRegister(driver, SX1272_REG_IRQ_FLAGS, SX1272_LORA_IRQ_TXDONE); // clears the IRQ flag
  SX1272_writeRegister(driver, SX1272_REG_FIFO_ADDR_PTR, 0x00);               // set pointer adddress to start
  // Load data into transmit FIFO
  for (int i = 0; i < length; i++) {
    SX1272_writeRegister(driver, SX1272_REG_FIFO, pointerdata[i]);
  }

  // Update the current operating mode
  _SX1272_setMode(driver, SX1272_MODE_TX); // Start transmitting
}

/* ============================================================================================== */
/**
 * @brief Begins continuous receive on the SX1272.
 *
 * @param lora Pointer to SX1272 struct.
 **
 * ============================================================================================== */
void SX1272_startReceive(LoRa_t *lora) {
  SX1272_t *driver = (SX1272_t *)lora;

  // Set device to standby
  _SX1272_setMode(driver, SX1272_MODE_STDBY);

  // TODO: add in proper read-mask-write operation for setting DIO mapping
  //
  // Set DIO interrupt pin to RxDone
  SX1272_writeRegister(driver, SX1272_REG_DIO_MAPPING1, SX1272_LORA_DIO_RXDONE);

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
  // SX1272_writeRegister(driver, SX1272_REG_IRQ_FLAGS, SX1272_LORA_IRQ_RXDONE); // Clear the IRQ flag
  SX1272_writeRegister(driver, SX1272_REG_FIFO_ADDR_PTR, 0x00); // Set pointer adddress to start

  // Update the current operating mode
  _SX1272_setMode(driver, SX1272_MODE_RXCONTINUOUS); // Start receiving
}

/* ============================================================================================== */
/**
 * @brief  Reads contents of received packet to local buffer from the SX1272.
 *
 * @param  lora     Pointer to SX1272 struct.
 * @param  buffer   Pointer to the buffer to store received data.
 * @param  buffSize Integer representing the size of the buffer to to fill.
 *
 * @return Boolean value indicating if a packet was successfully received and
 *         returned in buffer.
 **
 * ============================================================================================== */
uint8_t SX1272_readReceive(LoRa_t *lora, uint8_t *buffer, uint8_t buffSize) {
  SX1272_t *driver = (SX1272_t *)lora;

  // TODO: Error handling for IRQ flags
  //
  // Currently the readReceive() method clears the SX1272 RxDone IRQ flag before
  // starting the read. This is fine for cases where the user code carefully
  // manages the DIO interrupts, however ideally the method should check for
  // errors in the IRQ register and appropriately discard received packets.

  // Clear the IRQ flag
  SX1272_writeRegister(driver, SX1272_REG_IRQ_FLAGS, SX1272_LORA_IRQ_RXDONE);

  // Read address and packet width information of received data
  uint8_t bytesReceived = SX1272_readRegister(driver, SX1272_REG_RX_BYTES);          // Number of bytes received
  uint8_t rxCurrentAddr = SX1272_readRegister(driver, SX1272_REG_FIFO_RX_CURR_ADDR); // Address of last packet

  // Return error if buffer is smaller than the received data
  if (bytesReceived > buffSize) {
    return false;
  }

  // Otherwise, set the address pointer and read each byte into buffer
  SX1272_writeRegister(driver, SX1272_REG_FIFO_ADDR_PTR, rxCurrentAddr);
  for (int i = 0; i < bytesReceived; i++) {
    buffer[i] = SX1272_readRegister(driver, SX1272_REG_FIFO);
  }

  return bytesReceived;
}

/* ============================================================================================== */
/**
 * @brief  Sets the value of RegIrqFlags in the SX1272 to the provided argument value.
 *         Writing a 1 to a bit in the register will clear the associated flag.
 *
 * @param  lora  Pointer to SX1272 struct.
 * @param  flags 8-bit value representing flag bits to be set.
 *
 * @return @c NULL
 **
 * ============================================================================================== */
void SX1272_clearIRQ(LoRa_t *lora, uint8_t flags) {
  SX1272_writeRegister((SX1272_t *)lora, SX1272_REG_IRQ_FLAGS, flags);
}

/* ============================================================================================== */
/**
 * @brief   Update SX1272 configuration
 * @details
 *
 * @param   spi Pointer to SPI_t struct.
 *
 * @return  @c NULL.
 **
 * ============================================================================================== */
bool SX1272_updateConfig(SX1272_t *lora, SX1272_Config *config) {
  // Initialise config with default values if passed NULL.
  if (config == NULL) {
    config = &SX1272_CONFIG_DEFAULT;
  }

  // Validate max values
  if (config->outputPower > 0x1F || config->ocpTrim > 0x1F) {
    return false;
  }

  // Update peripheral with new config
  lora->config = *config;

  // Initialise SPI registers and enable peripheral
  _SX1272_init(lora, config);

  return true;
}

/*************************************** INTERFACE METHODS ****************************************/

// NOTE: This function is left exposed to public imports for debug purposes
uint8_t SX1272_readRegister(SX1272_t *lora, uint8_t address) {
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

void SX1272_writeRegister(SX1272_t *lora, uint8_t address, uint8_t data) {
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
