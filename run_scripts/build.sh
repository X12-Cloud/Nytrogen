#!/bin/bash
set -e

echo "--- Performing an incremental build of Nytrogen ---"

cmake --build ../build

echo "--- Incremental build complete ---"
