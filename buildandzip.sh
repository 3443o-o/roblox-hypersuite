#!/bin/bash
set -e

# =========================
# Configurable file lists
# =========================
# Files you want to keep in resources for each platform:
WINDOWS_FILES=("file1.txt" "file2.png")   # <-- change this
LINUX_FILES=("file1.txt" "file3.dat")     # <-- change this

# Build directories
BUILD_DIR="build"
WIN_DIR="$BUILD_DIR/win64"
LINUX_DIR="$BUILD_DIR/linux"
RESOURCE_DIR="resources"

# =========================
# Run make
# =========================
echo "Running make..."
make

# =========================
# Copy resources to build directories
# =========================
echo "Copying resources to build directories..."
mkdir -p "$WIN_DIR"
mkdir -p "$LINUX_DIR"

cp -r "$RESOURCE_DIR" "$WIN_DIR/"
cp -r "$RESOURCE_DIR" "$LINUX_DIR/"

# =========================
# Clean resources in Windows build
# =========================
echo "Cleaning Windows resources..."
cd "$WIN_DIR/$RESOURCE_DIR"
for f in *; do
    if [[ ! " ${WINDOWS_FILES[@]} " =~ " $f " ]]; then
        rm -rf "$f"
    fi
done
cd ../../../..

# =========================
# Clean resources in Linux build
# =========================
echo "Cleaning Linux resources..."
cd "$LINUX_DIR/$RESOURCE_DIR"
for f in *; do
    if [[ ! " ${LINUX_FILES[@]} " =~ " $f " ]]; then
        rm -rf "$f"
    fi
done
cd ../../../..

# =========================
# Zip the builds
# =========================
echo "Creating zip archives..."
cd "$BUILD_DIR"
zip -r utility-win64.zip win64/*
zip -r utility-linux-x86_64.zip linux/*
cd ..

echo "Done! Created utility-win64.zip and utility-linux-x86_64.zip"
