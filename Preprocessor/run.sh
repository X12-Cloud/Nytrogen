#!/bin/bash

if [ -z "$1" ]; then
    echo "Usage: $0 <input_file.nyt>"
    exit 1
fi

INPUT_FILE=$1

# Get the directory of the script
SCRIPT_DIR=$(dirname "$0")

# Run the preprocessor
"$SCRIPT_DIR/build/nytro-pre" "$INPUT_FILE"
