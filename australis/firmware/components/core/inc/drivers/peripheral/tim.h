/**
 * @author Matt Ricci
 * @addtogroup API API Reference
 * @{
 * @addtogroup TIM
 * @}
 */

// ALLOW FORMATTING
#ifndef TIM_H
#define TIM_H

#include "stm32f439xx.h"
#include "stdint.h"
#include "stdbool.h"

// Macro definitions for pin config literals
//

#define TIM_CONFIG_DEFAULT   \
  (TIM_Config) {             \
    .ARPE = false,           \
    .DIR  = TIM_DIR_UP,      \
    .OPM  = false,           \
    .URS  = TIM_URS_ANY,     \
    .UDIS = false,           \
    .CCDS = TIM_CCDS_UPDATE, \
    .CCDE = {},              \
    .UDE  = false,           \
    .CCIE = {},              \
    .UIE  = false            \
  }

// clang-format off

#define TIM_CR1_CONFIG_MASK ( \
    TIM_CR1_ARPE              \
  | TIM_CR1_DIR               \
  | TIM_CR1_OPM               \
  | TIM_CR1_URS               \
  | TIM_CR1_UDIS              \
)

#define TIM_CR2_CONFIG_MASK ( \
  TIM_CR2_CCDS                \
)

#define TIM_DIER_CONFIG_MASK ( \
    TIM_DIER_CC4DE             \
  | TIM_DIER_CC3DE             \
  | TIM_DIER_CC2DE             \
  | TIM_DIER_CC1DE             \
  | TIM_DIER_UDE               \
  | TIM_DIER_CC4IE             \
  | TIM_DIER_CC3IE             \
  | TIM_DIER_CC2IE             \
  | TIM_DIER_CC1IE             \
  | TIM_DIER_UDE               \
)

#define TIM_CCMR_CONFIG_MASK 0xFF

// clang-format on

/**
 * @ingroup TIM
 * @addtogroup TIM_Interface Interface
 * @brief TIM interface for
 * @{
 */

/**
 * @brief
 * @details
 */
typedef enum {
  TIM_CHANNEL1,
  TIM_CHANNEL2,
  TIM_CHANNEL3,
  TIM_CHANNEL4
} TIM_Channel;

/**
 * @brief
 * @details
 */
typedef enum {
  TIM_DIR_UP,
  TIM_DIR_DOWN
} TIM_Direction;

/**
 * @brief
 * @details
 */
typedef enum {
  TIM_URS_ANY,
  TIM_URS_WRAP
} TIM_UpdateSource;

/**
 * @brief
 * @details
 */
typedef enum {
  TIM_CCDS_CCX,
  TIM_CCDS_UPDATE
} TIM_DmaSelect;

/**
 * @brief
 * @details
 */
typedef enum {
  TIM_CCM_SELECT_OUTPUT,
  TIM_CCM_SELECT_IN1,
  TIM_CCM_SELECT_IN2,
  TIM_CCM_SELECT_TRC
} TIM_CCM_Selection;

/**
 * @brief
 * @details
 */
typedef enum {
  TIM_CCM_OUTPUT_MODE_FROZEN,
  TIM_CCM_OUTPUT_MODE_ACTIVE,
  TIM_CCM_OUTPUT_MODE_INACTIVE,
  TIM_CCM_OUTPUT_MODE_TOGGLE,
  TIM_CCM_OUTPUT_MODE_FORCE_INACTIVE,
  TIM_CCM_OUTPUT_MODE_FORCE_ACTIVE,
  TIM_CCM_OUTPUT_MODE_PWM_1,
  TIM_CCM_OUTPUT_MODE_PWM_2
} TIM_CCM_OutputMode;

/**
 * @brief
 * @details
 */
typedef struct {
  TIM_CCM_Selection S  : 2; //!<
  bool FE              : 1; //!<
  bool PE              : 1; //!<
  TIM_CCM_OutputMode M : 3; //!<
  bool CE              : 1; //!<
} TIM_CCMR_Output;

/**
 * @brief   TIM configuration struct
 * @details Describes the configuration of .
 */
typedef struct {
  bool ARPE;             //!<
  TIM_Direction DIR;     //!<
  bool OPM;              //!<
  TIM_UpdateSource URS;  //!<
  bool UDIS;             //!<
  TIM_DmaSelect CCDS;    //!<
  bool CCDE[4];          //!<
  bool CCIE[4];          //!<
  TIM_CCMR_Output OC[4]; //!<
  bool UDE;              //!<
  bool UIE;              //!<
} TIM_Config;

/**
 * @brief Struct definition for \ref TIM "TIM interface".
 * Provides the interface for API consumers to interact with the TIM peripheral.
 */
typedef struct TIM {
  TIM_TypeDef *interface;                                                 //!< Pointer to TIM interface struct.
  TIM_Config config;                                                      //!< Configuration parameters for the TIM peripheral.
  void (*startCounter)(struct TIM *tim);                                  //!< TIM configuration update method. @see TIM_updateConfig
  bool (*isRunning)(struct TIM *tim);                                     //!<
  bool (*pollUpdate)(struct TIM *tim);                                    //!<
  bool (*setTimingPeriod)(struct TIM *tim, float period);                 //!<
  bool (*pollCompare)(struct TIM *tim, TIM_Channel channel);              //!<
  bool (*setTimingPWM)(struct TIM *tim, TIM_Channel channel, float duty); //!<
  void (*updateConfig)(struct TIM *tim, TIM_Config *config);              //!< TIM configuration update method. @see TIM_updateConfig
} TIM_t;

TIM_t TIM_init(TIM_TypeDef *interface, TIM_Config *config);
void TIM_startCounter(TIM_t *tim);
bool TIM_isRunning(TIM_t *tim);
void TIM_updateConfig(TIM_t *tim, TIM_Config *config);
bool TIM_setTimingPWM(TIM_t *tim, TIM_Channel channel, float duty);
bool TIM_pollCompare(TIM_t *tim, TIM_Channel channel);
bool TIM_setTimingPeriod(TIM_t *tim, float period);
bool TIM_pollUpdate(TIM_t *tim);

/** @} */
#endif
