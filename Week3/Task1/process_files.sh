#!/bin/bash

# Input keyword
read -p "Please input the keyword: " KEYWORD

# Define result log file
RESULT_LOG="result.log"

# Clear the content of result_log
> "$RESULT_LOG"

# Initialize counter
file_count=0

# Find all the files with extension ".txt" and filter them
find . -type f -name "*.txt" | while read -r file; do
    # Check whether they contain the keyword
    if echo "$file" | grep -q "$KEYWORD"; then
        # Write the path to log
        echo "$file" >> "$RESULT_LOG"

        # Modify permission to "Read-only" (444)
        chmod 444 "$file"
    fi
done

file_count=$(wc -l < "$RESULT_LOG")

# Print "Completed" and the number of files handled
echo "Found $file_count files containing the keyword '$KEYWORD'. Permissions have been changed to read-only. Completed."

