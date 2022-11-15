aux_source_directory(. DIR_LIB_SRCS)

if (NOT CMAKE_CROSSCOMPILING)
    ## base test
    add_executable(poll_test poller_test.cpp)
    target_link_libraries(poll_test
                    base-io
                    base-util
                    ${GLOG_LIBRARY}
                    ${GFLAGS_LIBRARY}
                    gtest_main
                    pthread
                    )
    gtest_discover_tests(poll_test)
endif (NOT CMAKE_CROSSCOMPILING)
