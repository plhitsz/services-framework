aux_source_directory(. DIR_SRC_SRCS)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

project(base-src)
add_library (base-src ${DIR_SRC_SRCS})
target_link_libraries(base-src PUBLIC
    ${GLOG_LIBRARY}
    ${GFLAGS_LIBRARY}
    Threads::Threads)

    add_subdirectory(test)