/**
 * @file w25q128.h
 * @addtogroup Flash_W25Q128
 * @{
 */

// ALLOW FORMATTING
#ifndef _FLASH_H
#define _FLASH_H

#include "flash.h"

#include "spi.h"
#include "gpiopin.h"

#define W25Q128_PAGE_PROGRAM           0x02
#define W25Q128_READ_DATA              0x03
#define W25Q128_WRITE_ENABLE           0x06
#define W25Q128_ERASE_CHIP             0x60
#define W25Q128_READ_STATUS_REGISTER_1 0x05
#define W25Q128_READ_STATUS_REGISTER_2 0x35
#define W25Q128_READ_STATUS_REGISTER_3 0x15

#define W25Q128_STATUS1_BUSY           0x01
#define W25Q128_STATUS1_WEL            0x02

#define W25Q128_PAGE_SIZE              256   // 256 bytes per page
#define W25Q128_PAGE_COUNT             65536 // 65536 total pages

/**
 * @ingroup Flash
 * @addtogroup W25Q128
 * @{
 */

typedef struct W25Q128 {
  Flash_t base; //!< Base flash API
  SPI_t *spi;   //!< Parent SPI interface.
  GPIOpin_t cs; //!< Chip select GPIO.
} W25Q128_t;

W25Q128_t W25Q128_init(W25Q128_t *, SPI_t *, GPIOpin_t);
void W25Q128_readPage(Flash_t *, uint32_t, volatile uint8_t *);
void W25Q128_writePage(Flash_t *, uint32_t, uint8_t *);
void W25Q128_erase(Flash_t *);
void _W25Q128_writeEnable(W25Q128_t *);

/** @} */
#endif
