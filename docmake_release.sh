#!/bin/sh
export CC=gcc-8
export CXX=g++-8
cmake -DCMAKE_INSTALL_PREFIX="/usr/local/pinotams" -DCMAKE_BUILD_TYPE=Release ..
