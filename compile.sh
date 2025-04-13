#!/bin/bash

mkdir -p build
cd build
cmake ..
make
cd ..

cp OS/* build/

if [ $1 == "run" ]; then
    cd build
    sudo ./HOS
    cd ..
fi