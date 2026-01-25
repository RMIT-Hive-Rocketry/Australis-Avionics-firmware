cmake_minimum_required(VERSION 3.15)

project(Australis-Avionics-core LANGUAGES C ASM)

set(CMAKE_C_STANDARD 11)

add_library(Australis-core OBJECT)
target_compile_options(Australis-core PRIVATE
  -c -O0 -g3
)

# ╔══════════════════════════════════════════════════════════╗
# ║                       DEPENDENCIES                       ║
# ╚══════════════════════════════════════════════════════════╝

CPMAddPackage(
  NAME Australis-Avionics-lib
  URL  https://github.com/RMIT-Hive-Rocketry/Australis-Avionics-lib/archive/refs/heads/feature/cpm.zip
)

# Use Australis' default FreeRTOSConfig.h if no target already exists
if(NOT TARGET freertos_config)
  message(WARNING
    " No freertos_config target specified, defaulting to Australis provided configuration.\n"
    " NOTE: If you wish to use your own FreeRTOSConfig.h, it must be included in freertos_config.")

  # Add in config library
  add_library(freertos_config INTERFACE)

  target_include_directories(freertos_config SYSTEM
    INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/inc
  )
  target_compile_definitions(freertos_config
    INTERFACE
      projCOVERAGE_TEST=0
  )
endif()

# Select heap implementation
set(FREERTOS_HEAP "4" CACHE STRING "" FORCE)

# Select the ARM Cortex M4 port
set(FREERTOS_PORT "GCC_ARM_CM4F" CACHE STRING "" FORCE)

# Bring in CMSIS CORE
CPMAddPackage("gh:FreeRTOS/FreeRTOS-Kernel#V11.2.0")
set(FREERTOS ${FreeRTOS-Kernel_SOURCE_DIR}/ CACHE STRING "" FORCE)

# ╔══════════════════════════════════════════════════════════╗
# ║                         SOURCES                          ║
# ╚══════════════════════════════════════════════════════════╝


target_sources(Australis-core
  PRIVATE
    # -- SOURCE ---------------------------------------------
    ${CMAKE_CURRENT_LIST_DIR}/src/AustralisCore.c
    ${CMAKE_CURRENT_LIST_DIR}/src/devicelist.c
    ${CMAKE_CURRENT_LIST_DIR}/src/tasklist.c
    ${CMAKE_CURRENT_LIST_DIR}/src/state.c
    ${CMAKE_CURRENT_LIST_DIR}/src/topic.c
    ${CMAKE_CURRENT_LIST_DIR}/src/packet.c

    # -- RTOS -----------------------------------------------
    ${CMAKE_CURRENT_LIST_DIR}/src/rtos/stateupdate.c
    ${CMAKE_CURRENT_LIST_DIR}/src/rtos/flashwrite.c
    ${CMAKE_CURRENT_LIST_DIR}/src/rtos/messagedispatch.c
    ${CMAKE_CURRENT_LIST_DIR}/src/rtos/messageacquisition.c
    ${CMAKE_CURRENT_LIST_DIR}/src/rtos/data/hdataacquisition.c
    ${CMAKE_CURRENT_LIST_DIR}/src/rtos/data/ldataacquisition.c

    # -- PERIPHERAL DRIVERS ---------------------------------
    ${CMAKE_CURRENT_LIST_DIR}/src/drivers/peripheral/adc.c
    ${CMAKE_CURRENT_LIST_DIR}/src/drivers/peripheral/gpiopin.c
    ${CMAKE_CURRENT_LIST_DIR}/src/drivers/peripheral/spi.c
    ${CMAKE_CURRENT_LIST_DIR}/src/drivers/peripheral/tim.c
    ${CMAKE_CURRENT_LIST_DIR}/src/drivers/peripheral/can.c
    ${CMAKE_CURRENT_LIST_DIR}/src/drivers/peripheral/uart.c

    # -- SHELL ----------------------------------------------
    ${CMAKE_CURRENT_LIST_DIR}/src/shell/shell.c
    ${CMAKE_CURRENT_LIST_DIR}/src/shell/parser.c
    ${CMAKE_CURRENT_LIST_DIR}/src/shell/mem.c
    ${CMAKE_CURRENT_LIST_DIR}/src/shell/help.c
    ${CMAKE_CURRENT_LIST_DIR}/src/shell/clear.c
    $<TARGET_OBJECTS:Australis-system>
)
target_link_options(Australis-core BEFORE PUBLIC
  -T${CMAKE_CURRENT_LIST_DIR}/core.ld
)

# ╔══════════════════════════════════════════════════════════╗
# ║                         INCLUDES                         ║
# ╚══════════════════════════════════════════════════════════╝

add_library(Australis-core_headers INTERFACE)
target_include_directories(Australis-core_headers
  INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/inc
    ${CMAKE_CURRENT_LIST_DIR}/inc/shell
    ${CMAKE_CURRENT_LIST_DIR}/inc/rtos
    ${CMAKE_CURRENT_LIST_DIR}/inc/rtos/data
    ${CMAKE_CURRENT_LIST_DIR}/inc/drivers/peripheral
    ${CMAKE_CURRENT_LIST_DIR}/inc/drivers/device
    ${CMAKE_CURRENT_LIST_DIR}/inc/drivers/device/communication
    ${CMAKE_CURRENT_LIST_DIR}/inc/drivers/device/storage
    ${CMAKE_CURRENT_LIST_DIR}/inc/drivers/device/sensors
)
target_link_libraries(Australis-core_headers
  INTERFACE
    freertos_kernel
)

add_library(Australis-core_privateheaders INTERFACE)
target_include_directories(Australis-core_privateheaders
  INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/_inc/
)

target_link_libraries(Australis-core
  PUBLIC
    Australis-lib
    Australis-core_headers
  PRIVATE
    Australis-core_privateheaders
)
