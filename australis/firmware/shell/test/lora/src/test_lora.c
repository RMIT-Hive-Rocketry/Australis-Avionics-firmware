/***********************************************************************************
 * @file        testlora.c                                                         *
 * @author      Matt Ricci                                                         *
 * @addtogroup  Shell                   																					 *
 *                                                                                 *
 * @{                                                                              *
 ***********************************************************************************/

#if defined(EXTRA_uart) && defined(EXTRA_lora) && defined(EXTRA_sx1272)

#include "shell.h"
#include "topic.h"

#include "stdio.h"

#include "lorapub.h"
#include "uartpub.h"
#include "sx1272.h"

static void TestLora_exec(Shell *, uint8_t *);

DEFINE_PROGRAM_HANDLE("testlora", TestLora_exec)

/* =============================================================================== */
/**
 * @brief
 *
 *
 **
 * =============================================================================== */
static void TestLora_exec(Shell *shell, uint8_t *flags) {
  SX1272_t *lora = (SX1272_t *)LoRa_getTransceiver();

  CREATE_MESSAGE(m, 50)

  for (int i = 0; i < SX1272_REG_END; i++) {
    uint8_t regValue = SX1272_readRegister(lora, i);
    snprintf((char *)&m.data, 50, "SX1272 reg %#x: %#x", i, regValue);
    Topic_comment(uartTopic, (uint8_t *)&m);
  }
}

#endif

/** @} */
