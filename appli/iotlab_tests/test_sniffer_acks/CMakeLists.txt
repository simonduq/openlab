if(${PLATFORM_HAS_PHY})
    add_executable(test_sniffer_ack main sniffer)
    target_link_libraries(test_sniffer_ack platform)
endif(${PLATFORM_HAS_PHY})
