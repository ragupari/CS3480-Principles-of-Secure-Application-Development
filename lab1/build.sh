#!/bin/bash

# Check if argument is given
if [ -z "$1" ]; then
    echo "❌ Error: No filename provided."
    echo "Usage: $0 <filename_without_.c>"
    exit 1
fi

filename="$1"
src="${filename}.c"
out="${filename}"

echo "🗑️  Removing old binary..."
rm -f "$out"

echo "🛠️  Compiling $src..."
gcc "$src" -o "$out"

if [ $? -eq 0 ]; then
    echo "✅ Compilation successful: ./$out"
else
    echo "❌ Compilation failed."
fi
