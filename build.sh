#!/bin/bash

if [ "$1" = "test" ]; then
    mkdir -p .build-test; cd .build-test
    cmake -DBUILD_TEST=1 .. && make -j $(nproc) && ctest -R can --verbose
else
    source sdk/environment-setup-corei7-64-polytech-linux
    mkdir -p .build; cd .build
    cmake .. && make -j $(nproc)
fi
