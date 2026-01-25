/**
 * @file flash.h
 * @brief Defines the API for Flash memory storage.
 * @addtogroup API_flash
 * @{
 */

/**
 * @addtogroup Core
 * @{
 *    @addtogroup API_Storage Storage
 * @}
 */

// ALLOW FORMATTING
#ifndef FLASH_H
#define FLASH_H

#include "stdint.h"

typedef struct Flash {
  unsigned int pageSize;                                          //!< Number of bytes per page.
  unsigned long pageCount;                                        //!< Total number of pages.
  void (*erase)(struct Flash *);                                  //!< Chip erase method.
  void (*readPage)(struct Flash *, uint32_t, volatile uint8_t *); //!< Read page method.
  void (*writePage)(struct Flash *, uint32_t, uint8_t *);         //!< Write page method.
} Flash_t;

/** @} */
#endif
