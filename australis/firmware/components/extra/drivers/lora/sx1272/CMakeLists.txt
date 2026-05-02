set(MODULE_NAME Australis-extra_sx1272)

# ╔══════════════════════════════════════════════════════════╗
# ║                         SOURCES                          ║
# ╚══════════════════════════════════════════════════════════╝

add_library(${MODULE_NAME} OBJECT)
target_sources(${MODULE_NAME}
  PRIVATE
    src/sx1272.c
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
