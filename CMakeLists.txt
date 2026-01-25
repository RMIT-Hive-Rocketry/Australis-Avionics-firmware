cmake_minimum_required(VERSION 3.15)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CPM_SOURCE_CACHE .cache)

# ╔══════════════════════════════════════════════════════════╗
# ║                         PROJECT                          ║
# ╚══════════════════════════════════════════════════════════╝

include(cmake/GitVersion.cmake)
get_version_from_git()

project(Australis-Avionics LANGUAGES C ASM VERSION ${PROJECT_VERSION})

set(CMAKE_C_STANDARD 11)

include(cmake/CPM.cmake)
CPMAddPackage("gh:TheLartians/Ccache.cmake@1.2.5")

set(AUSTRALIS_DIR ${CMAKE_SOURCE_DIR}/australis)
set(AUSTRALIS_FIRMWARE_DIR ${AUSTRALIS_DIR}/firmware)
set(AUSTRALIS_DOCS_DIR ${AUSTRALIS_DIR}/documentation)

set(BUILD_ARTIFACT_DIR ${CMAKE_BINARY_DIR}/output)

# ╔══════════════════════════════════════════════════════════╗
# ║                       DOCUMENTATION                      ║
# ╚══════════════════════════════════════════════════════════╝

find_package(Doxygen)

if(Doxygen_FOUND)
  CPMAddPackage("gh:jothepro/doxygen-awesome-css@2.3.4")
  set(AWESOME_CSS_DIR ${doxygen-awesome-css_SOURCE_DIR})

  file(READ README.md MAINPAGE_CONTENTS)

  configure_file(${AUSTRALIS_DOCS_DIR}/Doxyfile.in ${BUILD_ARTIFACT_DIR}/docs/Doxyfile @ONLY)
  configure_file(${AUSTRALIS_DOCS_DIR}/mainpage.dox.in ${BUILD_ARTIFACT_DIR}/docs/mainpage.dox @ONLY)

  doxygen_add_docs(
    documentation
    COMMENT "Generating project documentation with custom Doxyfile"
    CONFIG_FILE ${BUILD_ARTIFACT_DIR}/docs/Doxyfile
  )
endif()

# ╔══════════════════════════════════════════════════════════╗
# ║                          BUILD                           ║
# ╚══════════════════════════════════════════════════════════╝

include(cmake/factory.cmake)

# Build the firmware
add_subdirectory(${AUSTRALIS_FIRMWARE_DIR})
