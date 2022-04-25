#!/bin/sh

export CC=/usr/bin/clang-10
export CXX=/usr/bin/clang++-10
export LLVM_DIR=/usr/lib/llvm-10/lib/cmake/llvm
rm -rf build
mkdir build
cd build
cmake ..
make
cd ..
