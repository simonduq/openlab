if (PLATFORM MATCHES "iotlab-.*")
    add_executable(sonar main)
    target_link_libraries(sonar platform mac_csma)
endif (PLATFORM MATCHES "iotlab-.*")
