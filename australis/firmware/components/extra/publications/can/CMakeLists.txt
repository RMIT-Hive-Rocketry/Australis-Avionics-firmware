set(MODULE_NAME Australis-extra_can)

# ╔══════════════════════════════════════════════════════════╗
# ║                         SOURCES                          ║
# ╚══════════════════════════════════════════════════════════╝

add_library(${MODULE_NAME} OBJECT)
target_sources(${MODULE_NAME}
  PRIVATE
    src/canpub.c
)
target_link_options(${MODULE_NAME} BEFORE PUBLIC
  "-T${CMAKE_CURRENT_LIST_DIR}/canpub.ld"
)

# ╔══════════════════════════════════════════════════════════╗
# ║                         INCLUDES                         ║
# ╚══════════════════════════════════════════════════════════╝

target_include_directories(${MODULE_NAME}
  PUBLIC
  inc/
  #TODO: fix this ugly hack that covers up error "devices.h not found" when compiling canpub target
  ${CMAKE_SOURCE_DIR}/australis/firmware/targets/${HARDWARE_TARGET}/master/inc
)

add_library(${MODULE_NAME}_headers INTERFACE)
target_include_directories(${MODULE_NAME}_headers
  INTERFACE
  inc/
)
