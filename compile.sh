#!/bin/bash

sudo rm -r os

mkdir -p .build
mkdir -p os

cd .build
cmake ..
make
cd ..

cp -r OS/* os/
cp .build/HOS os/

rm -r .build