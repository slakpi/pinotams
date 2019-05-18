#!/bin/sh
export CC=gcc-8
export CXX=g++-8
cmake -DCMAKE_INSTALL_PREFIX="/home/pi/Projects/pinotams" -DETC_PREFIX="./" -DVAR_PREFIX="./" -DCMAKE_BUILD_TYPE=Debug ..
