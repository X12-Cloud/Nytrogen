#!/bin/bash

# Clear the full_code.txt file if it already exists
> full_code.txt

echo "Processing files from 'src' directory..."

# Loop through all files in the 'src' directory
find src -type f | while read file; do
  echo "--- File: $file ---" >> full_code.txt
  cat "$file" >> full_code.txt
  echo "" >> full_code.txt # Add a newline after file content
  echo "" >> full_code.txt # Add an extra newline for better separation between files
done

echo "Processing files from 'include' directory..."

# Loop through all files in the 'include' directory
find include -type f | while read file; do
  echo "--- File: $file ---" >> full_code.txt
  cat "$file" >> full_code.txt
  echo "" >> full_code.txt # Add a newline after file content
  echo "" >> full_code.txt # Add an extra newline for better separation between files
done

echo "All done! Check full_code.txt"

