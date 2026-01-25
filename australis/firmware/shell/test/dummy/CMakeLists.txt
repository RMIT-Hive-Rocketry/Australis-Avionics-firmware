
if(ENABLE_DUMMY)
  # Create static library with data sources
  target_sources(Australis-test 
    PRIVATE
      ${CMAKE_CURRENT_LIST_DIR}/data/accel/accelX.c
      ${CMAKE_CURRENT_LIST_DIR}/data/accel/accelY.c
      ${CMAKE_CURRENT_LIST_DIR}/data/accel/accelZ.c
      ${CMAKE_CURRENT_LIST_DIR}/data/baro/press.c
      ${CMAKE_CURRENT_LIST_DIR}/data/gyro/gyroX.c
      ${CMAKE_CURRENT_LIST_DIR}/data/gyro/gyroY.c
      ${CMAKE_CURRENT_LIST_DIR}/data/gyro/gyroZ.c
      ${CMAKE_CURRENT_LIST_DIR}/src/test_dummy.c
  )
  # Add in test include directories
  target_include_directories(Australis-test 
    PUBLIC 
      ${CMAKE_CURRENT_LIST_DIR}/data/accel
      ${CMAKE_CURRENT_LIST_DIR}/data/baro
      ${CMAKE_CURRENT_LIST_DIR}/data/gyro
  )
  set(DUMMY -DDUMMY CACHE STRING "")
endif()
