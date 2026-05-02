set(MODULE_NAME Australis-extra_w25q128)

# ╔══════════════════════════════════════════════════════════╗
# ║                         SOURCES                          ║
# ╚══════════════════════════════════════════════════════════╝

add_library(${MODULE_NAME} OBJECT)
target_sources(${MODULE_NAME}
  PRIVATE
    src/w25q128.c
)

# ╔══════════════════════════════════════════════════════════╗
# ║                         INCLUDES                         ║
# ╚══════════════════════════════════════════════════════════╝

target_include_directories(${MODULE_NAME}
  PUBLIC
    inc/
)
