aux_source_directory(. DIR_IOLIB_SRCS)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

project(base-io)
add_library (base-io ${DIR_IOLIB_SRCS})
target_link_libraries(base-io PUBLIC
    ${GLOG_LIBRARY}
    ${GFLAGS_LIBRARY}
    Threads::Threads)
target_include_directories(base-io
    PUBLIC
       
        
)
add_subdirectory(test)