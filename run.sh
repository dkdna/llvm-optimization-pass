#!/bin/sh

clang-10 -S -emit-llvm -Xclang -disable-O0-optnone -O0 examples/$1 -o examples/foo-beforeopt.ll
opt -S -mem2reg examples/foo-beforeopt.ll -o examples/foo-beforeopt.ll
opt -load build/customopt/libCustomOptPass.so -dcelim -srcf -cse -S examples/foo-beforeopt.ll -o examples/foo-afteropt.ll
clang-10 -O0 examples/foo-afteropt.ll -o examples/foo
