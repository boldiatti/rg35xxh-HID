#!/bin/bash

# Create necessary directories
mkdir -p ${TARGET_DIR}/var/lib/bluetooth
mkdir -p ${TARGET_DIR}/etc/bluetooth
mkdir -p ${TARGET_DIR}/usr/lib/firmware/rtl_bt

# Set up Bluetooth firmware
if [ -f ${BR2_EXTERNAL_RG35XXH_PATH}/board/rg35xxh/rootfs_overlay/lib/firmware/rtl8723bs/rtl8723b_fw ]; then
    cp ${BR2_EXTERNAL_RG35XXH_PATH}/board/rg35xxh/rootfs_overlay/lib/firmware/rtl8723bs/* \
        ${TARGET_DIR}/usr/lib/firmware/rtl_bt/
fi

# Set permissions
chmod 755 ${TARGET_DIR}/usr/bin/rg35xxh-*
chmod 755 ${TARGET_DIR}/etc/init.d/S99controller

# Create minimal filesystem structure
ln -sf /proc/mounts ${TARGET_DIR}/etc/mtab

# Create device nodes (will be handled by devtmpfs)
# But create static nodes for early boot
mkdir -p ${TARGET_DIR}/dev
mknod -m 622 ${TARGET_DIR}/dev/console c 5 1
mknod -m 666 ${TARGET_DIR}/dev/null c 1 3
mknod -m 666 ${TARGET_DIR}/dev/zero c 1 5
mknod -m 666 ${TARGET_DIR}/dev/ptmx c 5 2
mknod -m 644 ${TARGET_DIR}/dev/random c 1 8
mknod -m 644 ${TARGET_DIR}/dev/urandom c 1 9

# Create device nodes for input
mkdir -p ${TARGET_DIR}/dev/input
for i in $(seq 0 5); do
    mknod -m 660 ${TARGET_DIR}/dev/input/event${i} c 13 $((${i}*64 + 64))
    mknod -m 660 ${TARGET_DIR}/dev/input/js${i} c 13 0
done

# Create framebuffer device
mknod -m 660 ${TARGET_DIR}/dev/fb0 c 29 0

# Create USB gadget device
mkdir -p ${TARGET_DIR}/dev/usb
mknod -m 660 ${TARGET_DIR}/dev/usb/gadget c 180 0

# Strip binaries to save space
find ${TARGET_DIR}/usr/bin -type f -executable -exec arm-linux-gnueabihf-strip {} \; 2>/dev/null
find ${TARGET_DIR}/usr/sbin -type f -executable -exec arm-linux-gnueabihf-strip {} \; 2>/dev/null
find ${TARGET_DIR}/lib -name "*.so*" -exec arm-linux-gnueabihf-strip {} \; 2>/dev/null

exit 0