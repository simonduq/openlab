include_directories(
    ${PROJECT_SOURCE_DIR}/drivers/cortex-m3
    ${PROJECT_SOURCE_DIR}/drivers/stm32
    ${PROJECT_SOURCE_DIR}/drivers/stm32f1xx
    )

# Create the printf library
add_library(iotlab_gpio STATIC iotlab_gpio)
add_library(iotlab_serial STATIC iotlab_serial)
add_library(iotlab_time STATIC iotlab_time)
add_library(iotlab_leds STATIC iotlab_leds)

