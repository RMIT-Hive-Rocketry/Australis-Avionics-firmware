cmake_minimum_required(VERSION 3.15)

project(Australis-Avionics-system LANGUAGES C ASM)

set(CMAKE_C_STANDARD 11)

add_library(Australis-system OBJECT)

# ╔══════════════════════════════════════════════════════════╗
# ║                       DEPENDENCIES                       ║
# ╚══════════════════════════════════════════════════════════╝

# Bring in CMSIS CORE
CPMAddPackage(
  NAME     CMSIS-CORE
  URL      https://github.com/ARM-software/CMSIS_5/archive/refs/tags/5.9.0.zip
  URL_HASH SHA256=3d4bfaf8783c633149510d098630a9d2f273dc090ea5e807103cc9b7acbb6708
)
set(CMSISCORE ${CMSIS-CORE_SOURCE_DIR}/CMSIS/Core CACHE STRING "" FORCE)

# Bring in CMSIS DSP
CPMAddPackage(
  NAME     CMSIS-DSP
  URL      https://github.com/ARM-software/CMSIS-DSP/archive/refs/tags/v1.16.2.zip
  URL_HASH SHA256=ded244d25b1b7ce4d75d6fcb9a25f8043caa007d0a355aee5dba6e7189ed4ccf
  DOWNLOAD_ONLY TRUE
)
set(CMSISDSP ${CMSIS-DSP_SOURCE_DIR}/ CACHE STRING "" FORCE)

# Bring in STM32F4 system files
CPMAddPackage(
  URI "gh:STMicroelectronics/cmsis-device-f4@2.6.11"
  PATCHES
    system_stm32f4xx.c.patch
    startup_stm32f439xx.s.patch
)
set(STM32F4 ${cmsis-device-f4_SOURCE_DIR}/ CACHE STRING "" FORCE)

# Retrieve SVD files from STM32F4 Device Family Pack
#
# The link for getting the SVD's directly from ST
# doesn't seem to work with CPM so we use what Keil
# provides instead
CPMAddPackage(
  NAME STM32F4-DFP
  URL "https://www.keil.com/pack/Keil.STM32F4xx_DFP.3.1.1.pack"
  DOWNLOAD_ONLY TRUE
)
set(STM32F4-SVD ${STM32F4-DFP_SOURCE_DIR}/CMSIS/SVD CACHE STRING "" FORCE)

# ╔══════════════════════════════════════════════════════════╗
# ║                         SOURCES                          ║
# ╚══════════════════════════════════════════════════════════╝

# Include startup assembly as separate library to avoid target compile
# options being added to the assembler flags.
add_library(Australis-system_asm OBJECT
  ${STM32F4}/Source/Templates/gcc/startup_stm32f439xx.s
)

target_sources(Australis-system
  PRIVATE
    ${CMSISDSP}/Source/BasicMathFunctions/BasicMathFunctions.c
    ${STM32F4}/Source/Templates/system_stm32f4xx.c
    ${CMAKE_CURRENT_LIST_DIR}/src/stubs.c
    ${CMAKE_CURRENT_LIST_DIR}/src/interrupts/interrupts.c
    ${CMAKE_CURRENT_LIST_DIR}/src/interrupts/interrupt_exti.c
    ${CMAKE_CURRENT_LIST_DIR}/src/interrupts/interrupt_spi.c
    ${CMAKE_CURRENT_LIST_DIR}/src/interrupts/interrupt_can.c
)

# ╔══════════════════════════════════════════════════════════╗
# ║                         INCLUDES                         ║
# ╚══════════════════════════════════════════════════════════╝

add_library(Australis-system_headers INTERFACE)

target_include_directories(Australis-system_headers
  INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/inc
    ${STM32F4}/Include
    ${CMSISCORE}/Include
    ${CMSISDSP}/Include
    ${CMSISDSP}/Include/dsp
    ${CMSISDSP}/PrivateInclude
)

add_library(Australis-system_privateheaders INTERFACE)
target_include_directories(Australis-system_privateheaders
  INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/_inc/
)

target_link_libraries(Australis-system
  PUBLIC
    Australis-system_headers
  PRIVATE
    Australis-system_privateheaders
    $<TARGET_OBJECTS:Australis-system_asm>
)
