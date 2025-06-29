#!/bin/bash

clear
set -e

# Clean up
sudo rm -rf os .build

mkdir -p .build
mkdir -p os

# Build your OS
cd .build
cmake ..
make
cd ..

# Copy OS files to 'os' folder (change if needed)
cp -r OS/* os/
cp .build/HOS os/

# Cleanup
#rm -rf .build
