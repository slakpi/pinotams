#!/bin/sh
export CC=gcc-9
export CXX=g++-9
cmake -DCMAKE_INSTALL_PREFIX=$(realpath ..) -DETC_PREFIX="./" -DVAR_PREFIX="./" -DCMAKE_BUILD_TYPE=Debug ..
