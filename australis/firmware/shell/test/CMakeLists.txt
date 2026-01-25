if(ENABLE_TEST)
  add_library(Australis-test OBJECT)

  add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/dummy)

  target_link_libraries(Australis-test
    PUBLIC
      Australis-core_headers
      Australis-extra_headers
    PRIVATE
      Australis-core
      Australis-core_privateheaders
  )

  target_link_libraries(Australis-firmware
    PRIVATE
      Australis-test
  )
  target_sources(Australis-firmware PUBLIC
    $<TARGET_OBJECTS:Australis-test>
  )
endif()
