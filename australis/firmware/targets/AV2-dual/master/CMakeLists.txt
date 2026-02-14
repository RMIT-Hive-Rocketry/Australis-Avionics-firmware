cmake_minimum_required(VERSION 3.15)

project(AV2-dual_master LANGUAGES C ASM)

# ╔══════════════════════════════════════════════════════════╗
# ║                          BUILD                           ║
# ╚══════════════════════════════════════════════════════════╝

build_australis_variant(
  NAME ${PROJECT_NAME}
  CONFIG_DIR "config"
  EXTRA_MODULES
    "drivers/sensors/accel/kx134_1211"
    "drivers/sensors/gyro/a3g4250d"
    "drivers/sensors/baro/bmp581"
    "drivers/flash/w25q128"
    "drivers/lora/sx1272"
    "drivers/gps/sam_m10q"
    "publications/lora"
    "publications/can"
  VARIANT_SOURCES
    "src/main.c"
    "src/devices.c"
    "src/tasks.c"
    "src/statelogic.c"
    "src/groundcomms.c"
  VARIANT_INCLUDES
    "inc"
)

file(MAKE_DIRECTORY ${MAP_DIR})
target_link_options(${PROJECT_NAME}
  PUBLIC
  LINKER:-Map=${MAP_DIR}/${PROJECT_NAME}.map
)
