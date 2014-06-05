# Add the test directory


if(${PLATFORM_HAS_INA226})
    include_directories(
    ${PROJECT_SOURCE_DIR}/drivers/stm32f1xx
    ${PROJECT_SOURCE_DIR}/drivers/cortex-m3
    ${PROJECT_SOURCE_DIR}/drivers/stm32
    ${PROJECT_SOURCE_DIR}/drivers/

    ${PROJECT_SOURCE_DIR}/appli/iotlab/periph
    ${PROJECT_SOURCE_DIR}/appli/iotlab/autotest_common
    ${PROJECT_SOURCE_DIR}/appli/iotlab/lib
    )
    add_executable(control_node
        main
        cn_control
        cn_consumption
        cn_radio
        cn_autotest
        cn_logger
        )
    target_link_libraries(control_node
        platform
        packet
        fiteco_lib_gwt
        iotlab_autotest_cn
        iotlab_serial
        iotlab_time
        iotlab_leds_util
        )
endif(${PLATFORM_HAS_INA226})
