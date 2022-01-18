#!/usr/bin/env bash

# A script that builds your compiler.
mkdir -p build 
cd build && cmake .. && make
