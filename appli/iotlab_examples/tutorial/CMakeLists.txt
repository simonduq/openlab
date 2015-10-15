include_directories(${PROJECT_SOURCE_DIR}/appli/iotlab/lib)

if (PLATFORM STREQUAL "iotlab-a8-m3")
add_executable(tutorial_a8_m3 main)
target_link_libraries(tutorial_a8_m3 platform iotlab_uid_num_table mac_csma)

elseif (PLATFORM STREQUAL "iotlab-m3")
add_executable(tutorial_m3 main)
target_link_libraries(tutorial_m3 platform lps331ap isl29020 iotlab_uid_num_table mac_csma)
endif (PLATFORM STREQUAL "iotlab-a8-m3")
