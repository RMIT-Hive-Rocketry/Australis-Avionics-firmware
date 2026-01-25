cmake_minimum_required(VERSION 3.15)

project(Australis-Avionics-components LANGUAGES C ASM)

add_library(Australis-components OBJECT)

# ╔══════════════════════════════════════════════════════════╗
# ║                        COMPONENTS                        ║
# ╚══════════════════════════════════════════════════════════╝

# --- Generate system component library ---
add_subdirectory(system)
# -----------------------------------------

# --- Generate core component library ---
add_subdirectory(core)

# Package system component into core
#
# This is kept private in case of future
# revisions that separate the system
# component out into a portable layer
target_link_libraries(Australis-core
  PRIVATE
    Australis-system
)
# -----------------------------------------

# ╔══════════════════════════════════════════════════════════╗
# ║                          OUTPUT                          ║
# ╚══════════════════════════════════════════════════════════╝

target_link_libraries(Australis-components
PUBLIC
  Australis-lib
  Australis-system_headers
  Australis-core_headers
PRIVATE
  Australis-system
  Australis-core
  Australis-core_privateheaders
)
target_sources(Australis-components PUBLIC
  $<TARGET_OBJECTS:Australis-system>
  $<TARGET_OBJECTS:Australis-core>
)
target_link_options(Australis-components PUBLIC
  $<TARGET_PROPERTY:Australis-core,LINK_OPTIONS>
)
