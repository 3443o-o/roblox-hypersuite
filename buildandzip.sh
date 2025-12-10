#!/bin/bash
set -e

# =========================
# Configurable file lists
# =========================
WINDOWS_REMOVE=("app.manifest" "app.rc" "app.res" "icon.o" "icon.rc" "icon.ico")
LINUX_REMOVE=("app.manifest" "app.rc" "app.res" "fumble.exe" "icon.o" "icon.rc" "WinDivert.dll" "WinDivert64.sys" "icon.ico")

BUILD_DIR="build"
WIN_DIR="$BUILD_DIR/win64"
LINUX_DIR="$BUILD_DIR/linux"
RESOURCE_DIR="resources"

# =========================
# Helper function for timing
# =========================
timer_start=$(date +%s)
function print_elapsed() {
    local start=$1
    local msg=$2
    local now=$(date +%s)
    local elapsed=$((now - start))
    echo "$msg (took ${elapsed}s)"
}

# =========================
# Build step
# =========================
echo "============================"
echo "Building LINUX target..."
start=$(date +%s)
make
print_elapsed $start "Linux build finished"
sleep 0.5

echo "Building Win64 target..."
start=$(date +%s)
make windows
print_elapsed $start "Win64 build finished"
sleep 0.5

# =========================
# Copy resources
# =========================
echo "============================"
echo "Copying resources to build directories..."
start=$(date +%s)
mkdir -p "$WIN_DIR" "$LINUX_DIR"
cp -r "$RESOURCE_DIR" "$WIN_DIR/"
cp -r "$RESOURCE_DIR" "$LINUX_DIR/"
print_elapsed $start "Resources copied"
sleep 0.5

# =========================
# Remove unwanted resources
# =========================
echo "Removing unwanted Linux resources..."
start=$(date +%s)
for f in "${LINUX_REMOVE[@]}"; do
    TARGET="$LINUX_DIR/$RESOURCE_DIR/$f"
    if [ -e "$TARGET" ]; then
        rm -rf "$TARGET"
        echo "  Removed $f from Linux build"
    fi
done
print_elapsed $start "Linux cleanup finished"
sleep 0.5

echo "Removing unwanted Windows resources..."
start=$(date +%s)
for f in "${WINDOWS_REMOVE[@]}"; do
    TARGET="$WIN_DIR/$RESOURCE_DIR/$f"
    if [ -e "$TARGET" ]; then
        rm -rf "$TARGET"
        echo "  Removed $f from Windows build"
    fi
done
print_elapsed $start "Windows cleanup finished"
sleep 0.5

# =========================
# Zip the builds
# =========================
echo "============================"
echo "Creating zip archives..."
start=$(date +%s)
cd "$BUILD_DIR"
zip -r utility-win64.zip win64/* > /dev/null
zip -r utility-linux-x86_64.zip linux/* > /dev/null
cd ..
print_elapsed $start "Zipping finished"

# Total time
total_elapsed=$(( $(date +%s) - timer_start ))
echo "============================"
echo "All done! Total time: ${total_elapsed}s"
echo "Created utility-win64.zip and utility-linux-x86_64.zip"
