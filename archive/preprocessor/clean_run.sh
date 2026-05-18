#!/bin/bash
set -e

# Navigate to the Preprocessor directory
cd "$(dirname "$0")"

# Perform a clean build
./build.sh

# Run the preprocessor
./run.sh "$1"
