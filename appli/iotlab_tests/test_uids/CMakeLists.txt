include_directories(${PROJECT_SOURCE_DIR}/appli/iotlab/lib)

add_executable(test_uids main)
target_link_libraries(test_uids iotlab_uid_num_table platform)
