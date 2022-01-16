#!/usr/bin/env bash

# Script to compile your source
mkdir -p build 
cd build && cmake .. && make 
