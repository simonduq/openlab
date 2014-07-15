if(${PLATFORM_HAS_PHY})
    add_executable(zep_sniffer main zep_sniffer gps_synced_clock)
    target_link_libraries(zep_sniffer platform)
endif(${PLATFORM_HAS_PHY})

include_directories(
${PROJECT_SOURCE_DIR}/drivers/stm32f1xx
${PROJECT_SOURCE_DIR}/drivers/cortex-m3
${PROJECT_SOURCE_DIR}/drivers/stm32
${PROJECT_SOURCE_DIR}/drivers/

${PROJECT_SOURCE_DIR}/appli/iotlab/lib
)
