cmake_minimum_required(VERSION 3.15)

project(Australis-Avionics-components LANGUAGES C ASM)

# ╔══════════════════════════════════════════════════════════╗
# ║                         PROJECT                          ║
# ╚══════════════════════════════════════════════════════════╝

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output/bin/${HARDWARE_TARGET})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output/lib/${HARDWARE_TARGET})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output/lib/${HARDWARE_TARGET})
set(MAP_DIR ${CMAKE_BINARY_DIR}/output/map/${HARDWARE_TARGET})
file(MAKE_DIRECTORY ${MAP_DIR})

# ╔══════════════════════════════════════════════════════════╗
# ║                          BUILD                           ║
# ╚══════════════════════════════════════════════════════════╝

# Build the combined components library
add_subdirectory(components)
add_subdirectory(shell/debug)
add_subdirectory(shell/test)

if(HARDWARE_TARGET AND NOT EXISTS ${CMAKE_CURRENT_LIST_DIR}/targets/${HARDWARE_TARGET})
  message(FATAL_ERROR
    " Target directory ${HARDWARE_TARGET} not found. Please specify a valid target."
  )
elseif(NOT HARDWARE_TARGET)
  message(WARNING
    " Configuring build without a defined hardware target.\n"
    " If you are trying to build a target from Australis-Avionics-firmware"
    " you must define HARDWARE_TARGET.\n"
    " If you are building a custom target this warning can be ignored."
  )
else()
  # Build the selected target
  add_subdirectory(targets/${HARDWARE_TARGET})
endif()


