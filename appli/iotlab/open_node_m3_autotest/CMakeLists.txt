include_directories(
     ${PROJECT_SOURCE_DIR}/drivers/cortex-m3
     ${PROJECT_SOURCE_DIR}/drivers/stm32
     ${PROJECT_SOURCE_DIR}/drivers/stm32f1xx
     ${PROJECT_SOURCE_DIR}/appli/iotlab/drivers
     ${PROJECT_SOURCE_DIR}/appli/iotlab/lib
     ${PROJECT_SOURCE_DIR}/appli/iotlab/autotest_common
     )


# Build for open nodes

if (PLATFORM STREQUAL "iotlab-m3" OR PLATFORM STREQUAL "iotlab-a8-m3")

add_executable(open_node_autotest main)
target_link_libraries(open_node_autotest  iotlab_autotest_on iotlab_gpio platform)

endif (PLATFORM STREQUAL "iotlab-m3" OR PLATFORM STREQUAL "iotlab-a8-m3")

