#!/bin/bash

# Ninja to make it compatible with vscode

# generate the build folder
cmake -G Ninja -B build

# build the project
cmake --build build -j 10 -t main
