if (PLATFORM STREQUAL "iotlab-a8-m3")

add_executable(sonar main)
target_link_libraries(sonar platform mac_csma)


else (PLATFORM STREQUAL "iotlab-a8-m3")
add_executable(sonar main)

target_link_libraries(sonar platform mac_csma) 
endif (PLATFORM STREQUAL "iotlab-a8-m3")












