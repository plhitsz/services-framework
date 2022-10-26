#!/bin/bash
echo "Start building submodule target ..."

CMAKE_BUILD_DIR=$1
CMAKE_SOURCE_DIR=$2

function build_git_src()
{
    local name=$1
    local cmake_param=$2
    echo "build ${name} with " $cmake_param
    if [ ! -d ${CMAKE_SOURCE_DIR}/submodules/${name}.git/build ]; then
        mkdir -p ${CMAKE_SOURCE_DIR}/submodules/${name}.git/build
    fi
    cd ${CMAKE_SOURCE_DIR}/submodules/${name}.git/build && \
        rm * -rf && \
        cmake -DCMAKE_INSTALL_PREFIX=${CMAKE_BUILD_DIR}/${name} ${cmake_param} .. && \
        make && make install
    echo "$name is install to ${CMAKE_BUILD_DIR}/${name}"
}

if [ ! -f ${CMAKE_BUILD_DIR}/gflags/lib/libgflags.a ]; then
    build_git_src gflags
else
    echo "skip to build gflags"
fi

if [ ! -f ${CMAKE_BUILD_DIR}/glog/lib/libglog.a -o ! -f ${CMAKE_BUILD_DIR}/glog/include/glog/logging.h ]; then
    build_git_src glog "-DBUILD_SHARED_LIBS=OFF"
else
    echo "skip to build glog"
fi

echo "Submodule target building is finished"
