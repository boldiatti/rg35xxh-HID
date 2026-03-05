#!/bin/bash
# build.sh - Main build script for RG35XXH Controller Firmware

set -e

# Configuration
BUILDROOT_VERSION="2023.02.6"
BUILDROOT_DIR="buildroot"
OUTPUT_DIR="output"
IMG_FILE="controller-os-rg35xxh.img"
DEFCONFIG_NAME="rg35xxh_controller_defconfig"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m'

echo -e "${GREEN}RG35XXH Controller Firmware Builder${NC}"
echo "======================================"

# Download Buildroot if not present
if [ ! -d "$BUILDROOT_DIR" ]; then
    echo "Downloading Buildroot $BUILDROOT_VERSION..."
    wget https://buildroot.org/downloads/buildroot-$BUILDROOT_VERSION.tar.gz
    tar xf buildroot-$BUILDROOT_VERSION.tar.gz
    mv buildroot-$BUILDROOT_VERSION "$BUILDROOT_DIR"
    rm buildroot-$BUILDROOT_VERSION.tar.gz
fi

# Copy our files to Buildroot
echo "Copying files to Buildroot..."
cp -r board "$BUILDROOT_DIR/"
cp -r package "$BUILDROOT_DIR/"
cp -r patches "$BUILDROOT_DIR/"
cp external.mk "$BUILDROOT_DIR/"
cp external.desc "$BUILDROOT_DIR/"
cp Config.in "$BUILDROOT_DIR/"

# Build
cd "$BUILDROOT_DIR"
make -j$(nproc)

if [ $? -eq 0 ]; then
    echo -e "${GREEN}Build successful!${NC}"
    cp output/images/sdcard.img "../$IMG_FILE"
    echo "Image file: $IMG_FILE"
else
    echo -e "${RED}Build failed!${NC}"
    exit 1
fi
