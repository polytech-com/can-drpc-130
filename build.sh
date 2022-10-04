#!/bin/bash

if [ "$1" = "test" ]; then
    mkdir -p .build-test && cd .build-test
    cmake -DBUILD_TEST=1 .. && cmake --build . -j $(nproc)

    ctest -R can --verbose
    lcov -q -c -d . -o total.info
    lcov -q -r total.info "*usr/include/*" "*catch2*" "*test*" -o coverage.info
    genhtml coverage.info
else
    mkdir -p .build && cd .build
    cmake .. && cmake --build . -j $(nproc)
fi
