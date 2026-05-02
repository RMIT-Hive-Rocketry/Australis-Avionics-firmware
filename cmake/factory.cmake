# Set a convenience variable for the path to the 'extra' components directory.
set(AUSTRALIS_EXTRA_DIR "${AUSTRALIS_FIRMWARE_DIR}/components/extra")

# Defines a "variant" of the final executable.
# A variant is defined by a NAME and a list of EXTRA_MODULES to include.
# This function creates an aggregate static library containing all specified
# extra modules and links it to the final executable.
#
# Usage:
#   build_australis_variant(
#     NAME MyVariantName
#     EXTRA_MODULES
#       module/submodule
#   )
function(build_australis_variant)

  set(oneValueArgs
    NAME
    CONFIG_DIR
  )

  set(multiValueArgs
    VARIANT_SOURCES
    VARIANT_INCLUDES
    EXTRA_MODULES
  )

  # -------------------------------------------------------------------------------------
  # ARGUMENT PARSING
  # -------------------------------------------------------------------------------------

  # Parse the arguments passed to this function (available in ${ARGN}).
  cmake_parse_arguments(ARG "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})


  if(NOT ARG_NAME)
    # The NAME argument is mandatory and is used for the final target name.
    message(FATAL_ERROR "build_australis_variant requires a NAME argument.")
  endif()

  if(NOT ARG_CONFIG_DIR)
    # The CONFIG_DIR argument is mandatory and is used for providing configuration
    # header include directories to Australis component libraries.
    message(FATAL_ERROR "build_australis_variant requires CONFIGURE_DIR to be defined.")
  endif()

  # -------------------------------------------------------------------------------------
  # CONFIGURE CORE COMPONENT AND TARGET VARIANT
  # -------------------------------------------------------------------------------------

  # Create interface library target for config headers
  # and add provided directory for includes
  add_library(config_${ARG_NAME} INTERFACE)
  target_include_directories(config_${ARG_NAME}
    INTERFACE
      ${ARG_CONFIG_DIR}
  )

  # Link config target into Australis-core to provide
  # configuration definitions.
  #
  # Since this property is public it will propogate upwards
  # to other targets linking to Australis-core
  target_link_libraries(Australis-core_headers INTERFACE config_${ARG_NAME})

  set(VARIANT_LIB Australis_${ARG_NAME})

  # Construct the variant executable from provided inputs
  add_executable(${ARG_NAME} ${ARG_VARIANT_SOURCES})
  target_include_directories(${ARG_NAME}
    PUBLIC
      ${ARG_VARIANT_INCLUDES}
  )

  # -------------------------------------------------------------------------------------
  # CONFIGURE EXTRA COMPONENT
  # -------------------------------------------------------------------------------------

  # --- Process Each Extra Module ---
  # Iterate over the list of extra modules provided in the EXTRA_MODULES argument.
  foreach(_MODULE ${ARG_EXTRA_MODULES})

    # Extract the module's name from its path (e.g., "publications/lora" -> "lora").
    # The `cmake_path` command is evaluated at configure-time and sets the variable
    # immediately for the logic that follows.
    cmake_path(GET _MODULE FILENAME _MODULE_NAME)
    set(_MODULE_LIB Australis-extra_${_MODULE_NAME})

    # Ensure each module is only added and configured once, even if requested
    # by multiple variants. This prevents duplicate target errors.
    if(NOT TARGET ${_MODULE_LIB})
      add_subdirectory(
        ${AUSTRALIS_EXTRA_DIR}/${_MODULE}
        ${CMAKE_BINARY_DIR}/firmware/components/extra/${_MODULE_NAME}
      )
      # Link the individual module library against core system libraries.
      target_link_libraries(${_MODULE_LIB}
        PUBLIC
          config_${ARG_NAME}
        PRIVATE
          Australis-components
          Australis-core_privateheaders
      )
    endif()

    # Add a compile definition (e.g., -DAUSTRALIS_EXTRA_console) to the variant
    # library. This allows for conditional compilation (`#ifdef`) in code that
    # depends on this module.
    target_compile_definitions(${ARG_NAME} INTERFACE "-DAUSTRALIS_EXTRA_${_MODULE_NAME}")

    # Link the individual module's library into the main variant aggregate library.
    # Using PUBLIC ensures that any public usage requirements from the module
    # are propagated to the final variant library.
    target_link_libraries(${ARG_NAME} PUBLIC ${_MODULE_LIB})

  endforeach()

  # Cache the name of the executable target. This allows other parts of the
  # CMake build system to easily reference the library created for this variant.
  set(CURRENT_TARGET_VARIANT ${ARG_NAME} CACHE INTERNAL "")

  # Link the final executable target against the components lib
  target_link_libraries(${ARG_NAME} PUBLIC Australis-components)

endfunction()
