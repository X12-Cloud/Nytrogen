#!/bin/bash
set -e

echo "--- Performing a clean build of Nytrogen ---"


echo "Removing existing 'build' directory..."
rm -rf ../build


echo "Configuring and building Nytrogen compiler..."
cmake -B ../build
cmake --build ../build

echo "--- Clean build complete ---"
