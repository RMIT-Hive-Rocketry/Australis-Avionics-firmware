cmake_minimum_required(VERSION 3.15)

project(Test-Units LANGUAGES C ASM)

# ╔══════════════════════════════════════════════════════════╗
# ║                          BUILD                           ║
# ╚══════════════════════════════════════════════════════════╝

build_australis_variant(
  NAME ${PROJECT_NAME}_BQueue
  CONFIG_DIR "config"
  EXTRA_MODULES
  VARIANT_SOURCES
    "src/test_bqueue.c"
  VARIANT_INCLUDES
    "inc"
)

file(MAKE_DIRECTORY ${MAP_DIR})
target_link_options(${PROJECT_NAME}_BQueue
  PUBLIC
  LINKER:-Map=${MAP_DIR}/${PROJECT_NAME}.map
)
