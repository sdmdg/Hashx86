#!/bin/bash

FIX_MODE=0
if [ "$1" == "--fix" ]; then
    FIX_MODE=1
    echo "--- Checking and FIXING End-of-File Newlines ---"
else
    echo "--- Checking End-of-File Newlines ---"
fi

ERROR=0

files=$(find . -type f \( -name "*.cpp" -o -name "*.c" -o -name "*.h" -o -name "*.asm" \))

for f in $files; do
    # Skip build/tool directories
    if [[ $f == *"./build"* ]]; then continue; fi
    if [[ $f == *"./tools"* ]]; then continue; fi
    if [[ -s "$f" && -n "$(tail -c 1 "$f")" ]]; then
        if [ $FIX_MODE -eq 1 ]; then
            # Append a newline
            echo "" >> "$f"
            echo "Fixed: $f"
        else
            echo "Missing Newline: $f"
            ERROR=1
        fi
    fi
done

if [ $FIX_MODE -eq 0 ]; then
    if [ $ERROR -eq 1 ]; then
        echo "FAILED: Some files are missing the final newline."
        exit 1
    else
        echo "All files look good."
        exit 0
    fi
fi
