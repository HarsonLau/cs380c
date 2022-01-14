#!/usr/bin/env bash

# Script to run your translator.
cd build && cmake .. && make &&cd ..
if test -f "$1"; then
    echo "$1 exists."
    ../src/csc $1 | ./build/lab1
fi
