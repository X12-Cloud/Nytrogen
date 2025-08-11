#!/bin/bash
# set -e ==> this works in the othe  rscripts but not here, dont really know why

echo "--- Performing a clean build and then running test.ny ---"

# Step 1: Perform a clean build using clean_run.sh
./run_scripts/clean_build.sh

# Step 2: Compile the Nytrogen source file using the freshly built compiler


# Step 3: run with run.sh
./run.sh
