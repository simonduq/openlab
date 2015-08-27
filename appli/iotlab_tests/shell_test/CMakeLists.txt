include_directories(
     ${PROJECT_SOURCE_DIR}/drivers/cortex-m3
     ${PROJECT_SOURCE_DIR}/drivers/stm32
     ${PROJECT_SOURCE_DIR}/drivers/stm32f1xx
     ${PROJECT_SOURCE_DIR}/appli/iotlab/periph
     )

add_executable(shell_test main)
target_link_libraries(shell_test shell platform)
