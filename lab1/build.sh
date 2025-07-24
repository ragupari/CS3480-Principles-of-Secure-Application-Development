#!/bin/bash

echo "🗑️  Removing old binary..."
rm -f lab1p1

echo "🛠️  Compiling..."
gcc lab1p1.c -o lab1p1

if [ $? -eq 0 ]; then
    echo "✅ Compilation successful: ./lab1p1"
else
    echo "❌ Compilation failed."
fi
