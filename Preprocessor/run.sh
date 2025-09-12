#!/bin/bash

if [ -z "$1" ]; then
    echo "Usage: $0 <input_file.nyt>"
    exit 1
fi

INPUT_FILE=$(readlink -f "$1")

# Navigate to the Preprocessor directory
cd "$(dirname "$0")"

# Run the preprocessor
./build/nytro-pre "$INPUT_FILE"
