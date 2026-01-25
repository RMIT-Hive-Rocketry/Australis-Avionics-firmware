set(MODULE_NAME Australis-extra_lora)

# ╔══════════════════════════════════════════════════════════╗
# ║                         SOURCES                          ║
# ╚══════════════════════════════════════════════════════════╝

add_library(${MODULE_NAME} OBJECT)
target_sources(${MODULE_NAME}
  PRIVATE
    src/lorapub.c
)
target_link_options(${MODULE_NAME} BEFORE PUBLIC
  "-T${CMAKE_CURRENT_LIST_DIR}/lorapub.ld"
)

# ╔══════════════════════════════════════════════════════════╗
# ║                         INCLUDES                         ║
# ╚══════════════════════════════════════════════════════════╝

target_include_directories(${MODULE_NAME}
  PUBLIC
    inc/
)

add_library(${MODULE_NAME}_headers INTERFACE)
target_include_directories(${MODULE_NAME}_headers
  INTERFACE
    inc/
)
