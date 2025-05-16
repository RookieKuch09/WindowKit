#!/bin/zsh

configure()
{
    cmake -B build \
          -G Ninja \
          -DCMAKE_CXX_COMPILER=clang++ \
          -DCMAKE_BUILD_TYPE=Debug \
          -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
          -DCMAKE_EXE_LINKER_FLAGS=-fuse-ld=lld \
          -S .
}

build()
{
    cmake --build build
}

install()
{
    sudo cmake --install build
}

configure && build && install