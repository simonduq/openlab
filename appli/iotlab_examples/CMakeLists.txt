add_subdirectory(serial_echo)
add_subdirectory(simple_idle)
add_subdirectory(serial_helo)
add_subdirectory(tutorial)

if(${PLATFORM_HAS_LSM303DLHC})
add_subdirectory(ping_pong)
endif(${PLATFORM_HAS_LSM303DLHC})
