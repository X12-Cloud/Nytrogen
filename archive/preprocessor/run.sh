#!/bin/bash

if [ -z "$1" ]; then
    echo "Usage: $0 <input_file.nyt>"
    exit 1
fi

INPUT_FILE=$1

# Get the absolute path of the script's directory
SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &> /dev/null && pwd)

# Run the preprocessor
"$SCRIPT_DIR/build/nytro-pre" "$INPUT_FILE"