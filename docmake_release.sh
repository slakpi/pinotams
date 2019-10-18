#!/bin/sh
export CC=gcc-9
export CXX=g++-9
cmake -DCMAKE_INSTALL_PREFIX="/usr/local/pinotams" -DCMAKE_BUILD_TYPE=Release ..

