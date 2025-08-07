#!/bin/bash

# Output binary name
OUTPUT="simulator"

# Source folder
SRC_DIR="user-group-management"
# Virtual File System folder
VFS_DIR="virtual-file-system"

AUDIT_DIR="audit"

# Source files
SRC_FILES="main.c $SRC_DIR/user.c $SRC_DIR/group.c $SRC_DIR/usermod.c $VFS_DIR/vfs.c $AUDIT_DIR/audit.c"

# Delete previous binary if it exists
if [ -f "$OUTPUT" ]; then
    echo "Removing old build: $OUTPUT"
    rm "$OUTPUT"
fi

# Compile the sources
echo "Building..."
gcc -Wall -Wextra $SRC_FILES -o $OUTPUT

# Build result
if [ $? -eq 0 ]; then
    echo "Build successful. Run with ./$OUTPUT"
else
    echo "Build failed."
fi
