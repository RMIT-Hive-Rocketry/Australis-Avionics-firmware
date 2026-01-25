cmake_minimum_required(VERSION 3.15)

project(AV2-dual_supervisor LANGUAGES C ASM)

# ╔══════════════════════════════════════════════════════════╗
# ║                          BUILD                           ║
# ╚══════════════════════════════════════════════════════════╝

build_australis_variant(
  NAME ${PROJECT_NAME}
  CONFIG_DIR "config"
  EXTRA_MODULES
    "drivers/sensors/accel/ais3624dq"
    "drivers/sensors/gyro/iam_20380"
    "drivers/sensors/baro/lps22df"
    "drivers/flash/w25q128"
#   "publications/can"
  VARIANT_SOURCES
    "src/main.c"
    "src/devices.c"
    "src/tasks.c"
    "src/statelogic.c"
  VARIANT_INCLUDES
    "inc"
)

file(MAKE_DIRECTORY ${MAP_DIR})
target_link_options(${PROJECT_NAME}
  PUBLIC
  LINKER:-Map=${MAP_DIR}/${PROJECT_NAME}.map
)
