## base test
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
#### node test
bats_test(node_test
    SRCS 
        node_test.cc
    DEPENDS
        base-src
        base-io
        base-util
        bats::library
        gtest_main
        ${GLOG_LIBRARY}
        ${GFLAGS_LIBRARY}
        Threads::Threads
        )