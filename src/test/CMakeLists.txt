## base test
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(coder_disorder_test coder_disorder_test.cc)
target_link_libraries(coder_disorder_test
                PRIVATE
                base-util
                base-io
                bats::library
                gtest_main
                ${GLOG_LIBRARY}
                ${GFLAGS_LIBRARY}
                Threads::Threads
                )
gtest_discover_tests(coder_disorder_test)

