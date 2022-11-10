aux_source_directory(. DIR_LIB_SRCS)

if (NOT CMAKE_CROSSCOMPILING)
    ## base test
    add_executable(settings_test settings_test.cpp)
    target_link_libraries(settings_test
                    gtest_main
                    base-util
                    pthread
                    )

    add_executable(seq_test seq_test.cpp)
    target_link_libraries(seq_test
                    gtest_main
                    base-util
                    pthread
                    )
    gtest_discover_tests(settings_test)
    gtest_discover_tests(seq_test)

    add_executable(timer_test timer_test.cc)
    target_link_libraries(timer_test
                    PRIVATE
                    base-util
                    base-io
                    bats::library
                    gtest_main
                    ${GLOG_LIBRARY}
                    ${GFLAGS_LIBRARY}
                    Threads::Threads
                    )
    gtest_discover_tests(timer_test)
endif (NOT CMAKE_CROSSCOMPILING)
