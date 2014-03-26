# Add the tasks directory
# iotlab specific code

if (PLATFORM MATCHES "fiteco-.*")

add_subdirectory(control_node)
add_subdirectory(idle_with_dc)

add_subdirectory(open_node_m3_autotest)

# Libraries
add_subdirectory(autotest_common)
add_subdirectory(lib)

endif (PLATFORM MATCHES "fiteco-.*")
