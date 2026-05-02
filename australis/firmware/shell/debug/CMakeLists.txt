if(ENABLE_DEBUG)
    add_library(Australis-debug OBJECT)
    target_compile_options(Australis-debug PRIVATE
        -c -O0 -g3
    )

    target_sources(Australis-debug
        PUBLIC
            $<TARGET_OBJECTS:Australis-system>
            $<TARGET_OBJECTS:Australis-core>
        PRIVATE
            ${CMAKE_CURRENT_LIST_DIR}/src/debug_adcvsense.c
            ${CMAKE_CURRENT_LIST_DIR}/src/debug_accelerometer.c
            ${CMAKE_CURRENT_LIST_DIR}/src/debug_barometer.c
            ${CMAKE_CURRENT_LIST_DIR}/src/debug_gyroscope.c
            ${CMAKE_CURRENT_LIST_DIR}/src/debug_launch.c
            ${CMAKE_CURRENT_LIST_DIR}/src/debug_state.c
            ${CMAKE_CURRENT_LIST_DIR}/src/debug_stack.c
    )

    if(TARGET Australis-extra_sx1272)
    target_sources(Australis-debug
        PRIVATE
            ${CMAKE_CURRENT_LIST_DIR}/src/debug_sx1272.c
    )
    endif()


    target_link_libraries(Australis-debug
        PUBLIC
            Australis-core
            Australis-system
    )
    target_sources(Australis-components
        PUBLIC
            $<TARGET_OBJECTS:Australis-debug>
    )
endif()

