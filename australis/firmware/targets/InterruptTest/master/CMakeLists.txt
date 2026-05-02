cmake_minimum_required(VERSION 3.15)

project(InterruptTest_master LANGUAGES C ASM)

# ╔══════════════════════════════════════════════════════════╗
# ║                          BUILD                           ║
# ╚══════════════════════════════════════════════════════════╝

build_australis_variant(
  NAME ${PROJECT_NAME}
  CONFIG_DIR "config"
  EXTRA_MODULES
  VARIANT_SOURCES
    "src/main.c"
  VARIANT_INCLUDES
    "inc"
)

file(MAKE_DIRECTORY ${MAP_DIR})
target_link_options(${PROJECT_NAME}
  PUBLIC
  LINKER:-Map=${MAP_DIR}/${PROJECT_NAME}.map
)
