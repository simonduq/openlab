add_subdirectory(tests_pps_nico)
add_subdirectory(test_control_node_absolute_time)
add_subdirectory(test_scanf_stack_usage)
add_subdirectory(test_serial_flood)
add_subdirectory(oml)
add_subdirectory(test_uids)

if (PLATFORM MATCHES "iotlab-.*")
    # Requires the 'iotlab-' libs
    add_subdirectory(m3_node_interract_cn)
endif (PLATFORM MATCHES "iotlab-.*")
