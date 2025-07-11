#!/bin/bash

mkdir -p build
cd build

cmake ..
cmake --build . --config Release

echo "Executable has been placed in the parent directory."