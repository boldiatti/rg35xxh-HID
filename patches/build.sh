#!/bin/bash
# build.sh - with GitHub Actions support

set -e

# GitHub Actions specific settings
if [ -n "$GITHUB_WORKSPACE" ]; then
    echo "Running in GitHub Actions"
    export FORCE_UNSAFE_CONFIGURE=1
    export DEBIAN_FRONTEND=noninteractive
fi

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

# Check for required packages
echo "Checking required packages..."
REQUIRED_PKGS="make gcc g++ bc bison flex libssl-dev device-tree-compiler python3"
MISSING_PKGS=""

for pkg in $REQUIRED_PKGS; do
    if ! dpkg -l | grep -q "^ii  $pkg"; then
        MISSING_PKGS="$MISSING_PKGS $pkg"
    fi
done

if [ ! -z "$MISSING_PKGS" ]; then
    echo -e "${RED}Missing required packages: $MISSING_PKGS${NC}"
    echo "Install with: sudo apt install $MISSING_PKGS"
    exit 1
fi

# Download Buildroot if not present
if [ ! -d "$BUILDROOT_DIR" ]; then
    echo "Downloading Buildroot $BUILDROOT_VERSION..."
    wget "https://buildroot.org/downloads/buildroot-$BUILDROOT_VERSION.tar.gz"
    tar xf "buildroot-$BUILDROOT_VERSION.tar.gz"
    mv "buildroot-$BUILDROOT_VERSION" "$BUILDROOT_DIR"
    rm "buildroot-$BUILDROOT_VERSION.tar.gz"
fi

# Create external tree structure
echo "Creating Buildroot external tree..."
mkdir -p "$BUILDROOT_DIR/board/rg35xxh/rootfs_overlay"

# Copy our files to Buildroot external tree
cp -r board/* "$BUILDROOT_DIR/board/"
cp -r package/* "$BUILDROOT_DIR/package/"
cp external.mk "$BUILDROOT_DIR/"
cp external.desc "$BUILDROOT_DIR/"
cp Config.in "$BUILDROOT_DIR/"

# Apply kernel patches
echo "Applying kernel patches..."
mkdir -p "$BUILDROOT_DIR/board/rg35xxh/patches/linux"
cp patches/linux/*.patch "$BUILDROOT_DIR/board/rg35xxh/patches/linux/"

# Create defconfig
echo "Creating defconfig..."
cat > "$BUILDROOT_DIR/configs/$DEFCONFIG_NAME" << 'EOF'
# Architecture
BR2_aarch64=y
BR2_cortex_a53=y
BR2_ARM_FPU_VFPV4=y

# Toolchain
BR2_TOOLCHAIN_BUILDROOT=y
BR2_TOOLCHAIN_BUILDROOT_GLIBC=y
BR2_TOOLCHAIN_BUILDROOT_CXX=y

# Kernel
BR2_LINUX_KERNEL=y
BR2_LINUX_KERNEL_CUSTOM_VERSION=y
BR2_LINUX_KERNEL_CUSTOM_VERSION_VALUE="6.1.31"
BR2_LINUX_KERNEL_USE_DEFCONFIG=y
BR2_LINUX_KERNEL_DEFCONFIG="sunxi"
BR2_LINUX_KERNEL_CONFIG_FRAGMENT_FILES="board/rg35xxh/linux.config"
BR2_LINUX_KERNEL_DTS_SUPPORT=y
BR2_LINUX_KERNEL_INTREE_DTS_NAME="allwinner/sun50i-h700-rg35xxh"

# Bootloaders
BR2_TARGET_UBOOT=y
BR2_TARGET_UBOOT_BUILD_SYSTEM_KCONFIG=y
BR2_TARGET_UBOOT_CUSTOM_VERSION=y
BR2_TARGET_UBOOT_CUSTOM_VERSION_VALUE="2023.07"
BR2_TARGET_UBOOT_BOARD_DEFCONFIG="rg35xxh"
BR2_TARGET_UBOOT_NEEDS_DTC=y
BR2_TARGET_UBOOT_NEEDS_ATF_BL31=y
BR2_TARGET_UBOOT_NEEDS_OPENSBI=n

# ARM Trusted Firmware
BR2_TARGET_ARM_TRUSTED_FIRMWARE=y
BR2_TARGET_ARM_TRUSTED_FIRMWARE_CUSTOM_VERSION=y
BR2_TARGET_ARM_TRUSTED_FIRMWARE_CUSTOM_VERSION_VALUE="v2.8"
BR2_TARGET_ARM_TRUSTED_FIRMWARE_PLATFORM="sun50i_h6"
BR2_TARGET_ARM_TRUSTED_FIRMWARE_BL31=y

# Filesystem
BR2_TARGET_ROOTFS_EXT2=y
BR2_TARGET_ROOTFS_EXT2_4=y
BR2_TARGET_ROOTFS_EXT2_SIZE="512M"
BR2_TARGET_GENIMAGE_EXTRA_CONFIG="board/rg35xxh/genimage.cfg"

# Packages
BR2_PACKAGE_BUSYBOX_CONFIG_FRAGMENT_FILES="board/rg35xxh/busybox.config"
BR2_PACKAGE_ALSA_LIB=y
BR2_PACKAGE_ALSA_UTILS=y
BR2_PACKAGE_BLUEZ5_UTILS=y
BR2_PACKAGE_BLUEZ5_UTILS_CLIENT=y
BR2_PACKAGE_BLUEZ5_UTILS_TOOLS=y
BR2_PACKAGE_EVTEST=y
BR2_PACKAGE_SDL2=y
BR2_PACKAGE_SDL2_FBCON=y
BR2_PACKAGE_LIBUSB=y
BR2_PACKAGE_LIBEVDEV=y
BR2_PACKAGE_READLINE=y
BR2_PACKAGE_DBUS=y
BR2_PACKAGE_DBUS_GLIB=y

# Custom packages
BR2_PACKAGE_RG35XXH_CONTROLLER=y
BR2_PACKAGE_RG35XXH_UI=y

# System configuration
BR2_TARGET_GENERIC_HOSTNAME="rg35xxh-controller"
BR2_TARGET_GENERIC_ISSUE="RG35XXH Controller Firmware"
BR2_SYSTEM_DHCP="eth0"
BR2_ROOTFS_OVERLAY="board/rg35xxh/rootfs_overlay"
BR2_ROOTFS_POST_BUILD_SCRIPT="board/rg35xxh/post-build.sh"

# Minimize size
BR2_ENABLE_DEBUG=n
BR2_STRIP_strip=y
BR2_OPTIMIZE_2=y
BR2_SHARED_STATIC_LIBS=n
EOF

# Build
echo "Starting build process..."
cd "$BUILDROOT_DIR"
make $DEFCONFIG_NAME
make -j$(nproc)

if [ $? -eq 0 ]; then
    echo -e "${GREEN}Build successful!${NC}"
    echo "Creating SD card image..."
    
    # Copy the generated image
    cp output/images/sdcard.img "../$IMG_FILE"
    
    echo -e "${GREEN}====================================${NC}"
    echo -e "${GREEN}Firmware built successfully!${NC}"
    echo "Image file: $IMG_FILE"
    echo ""
    echo "To flash to SD card:"
    echo "  sudo dd if=$IMG_FILE of=/dev/sdX bs=4M status=progress"
    echo "  sync"
    echo ""
    echo "Default button combinations:"
    echo "  SELECT + START  - Toggle USB gadget"
    echo "  SELECT + L1     - Bluetooth pairing mode"
    echo "  SELECT + R1     - Toggle Bluetooth visibility"
    echo "  SELECT + DPAD   - Switch between USB/Bluetooth modes"
    echo -e "${GREEN}====================================${NC}"
else
    echo -e "${RED}Build failed!${NC}"
    exit 1
fi