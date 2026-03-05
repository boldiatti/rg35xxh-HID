#include "usb_gadget.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

#define GADGET_BASE "/sys/kernel/config/usb_gadget"
#define GADGET_NAME "g1"

// HID Report Descriptor for Gamepad
static const uint8_t gamepad_hid_report_desc[] = {
    0x05, 0x01,        // Usage Page (Generic Desktop)
    0x09, 0x05,        // Usage (Game Pad)
    0xA1, 0x01,        // Collection (Application)
    0x85, 0x01,        //   Report ID (1)
    
    // Buttons (24 buttons)
    0x05, 0x09,        //   Usage Page (Button)
    0x19, 0x01,        //   Usage Minimum (Button 1)
    0x29, 0x18,        //   Usage Maximum (Button 24)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x01,        //   Logical Maximum (1)
    0x75, 0x01,        //   Report Size (1)
    0x95, 0x18,        //   Report Count (24)
    0x81, 0x02,        //   Input (Data,Var,Abs)
    
    // Hat Switch (4 bits)
    0x05, 0x01,        //   Usage Page (Generic Desktop)
    0x09, 0x39,        //   Usage (Hat Switch)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x07,        //   Logical Maximum (7)
    0x35, 0x00,        //   Physical Minimum (0)
    0x46, 0x3B, 0x01,  //   Physical Maximum (315)
    0x65, 0x14,        //   Unit (Degrees)
    0x75, 0x04,        //   Report Size (4)
    0x95, 0x01,        //   Report Count (1)
    0x81, 0x42,        //   Input (Data,Var,Abs,Null)
    
    // Padding to byte align
    0x75, 0x04,        //   Report Size (4)
    0x95, 0x01,        //   Report Count (1)
    0x81, 0x03,        //   Input (Const,Var,Abs)
    
    // Analog sticks (4 bytes)
    0x05, 0x01,        //   Usage Page (Generic Desktop)
    0x09, 0x30,        //   Usage (X)
    0x09, 0x31,        //   Usage (Y)
    0x09, 0x32,        //   Usage (Z)
    0x09, 0x35,        //   Usage (Rz)
    0x15, 0x00,        //   Logical Minimum (0)
    0x26, 0xFF, 0x00,  //   Logical Maximum (255)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x04,        //   Report Count (4)
    0x81, 0x02,        //   Input (Data,Var,Abs)
    
    // Triggers (2 bytes)
    0x09, 0x36,        //   Usage (Slider)
    0x09, 0x36,        //   Usage (Slider)
    0x15, 0x00,        //   Logical Minimum (0)
    0x26, 0xFF, 0x00,  //   Logical Maximum (255)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x02,        //   Report Count (2)
    0x81, 0x02,        //   Input (Data,Var,Abs)
    
    0xC0               // End Collection
};

static int write_file(const char *path, const char *value) {
    FILE *f = fopen(path, "w");
    if (!f) {
        return -1;
    }
    fprintf(f, "%s", value);
    fclose(f);
    return 0;
}

int usb_gadget_init(usb_gadget_t *gadget) {
    memset(gadget, 0, sizeof(usb_gadget_t));
    
    snprintf(gadget->gadget_path, sizeof(gadget->gadget_path),
             "%s/%s", GADGET_BASE, GADGET_NAME);
    
    return 0;
}

int usb_gadget_enable(usb_gadget_t *gadget) {
    char path[512];
    
    // Create gadget directory
    mkdir(gadget->gadget_path, 0755);
    
    // Set vendor and product IDs (Anbernic vendor ID, generic gamepad product ID)
    snprintf(path, sizeof(path), "%s/idVendor", gadget->gadget_path);
    write_file(path, "0x1949");
    
    snprintf(path, sizeof(path), "%s/idProduct", gadget->gadget_path);
    write_file(path, "0x0401");
    
    // Set device release number
    snprintf(path, sizeof(path), "%s/bcdDevice", gadget->gadget_path);
    write_file(path, "0x0100");
    
    // Set USB version
    snprintf(path, sizeof(path), "%s/bcdUSB", gadget->gadget_path);
    write_file(path, "0x0200");
    
    // Create strings
    snprintf(path, sizeof(path), "%s/strings/0x409", gadget->gadget_path);
    mkdir(path, 0755);
    
    snprintf(path, sizeof(path), "%s/strings/0x409/serialnumber", gadget->gadget_path);
    write_file(path, "RG35XXH001");
    
    snprintf(path, sizeof(path), "%s/strings/0x409/manufacturer", gadget->gadget_path);
    write_file(path, "Anbernic");
    
    snprintf(path, sizeof(path), "%s/strings/0x409/product", gadget->gadget_path);
    write_file(path, "RG35XXH Gamepad");
    
    // Create HID function
    snprintf(path, sizeof(path), "%s/functions/hid.usb0", gadget->gadget_path);
    mkdir(path, 0755);
    
    // Set HID protocol
    snprintf(path, sizeof(path), "%s/functions/hid.usb0/protocol", gadget->gadget_path);
    write_file(path, "0");
    
    snprintf(path, sizeof(path), "%s/functions/hid.usb0/subclass", gadget->gadget_path);
    write_file(path, "0");
    
    snprintf(path, sizeof(path), "%s/functions/hid.usb0/report_length", gadget->gadget_path);
    write_file(path, "10");
    
    // Write report descriptor
    snprintf(path, sizeof(path), "%s/functions/hid.usb0/report_desc", gadget->gadget_path);
    FILE *f = fopen(path, "w");
    if (f) {
        fwrite(gamepad_hid_report_desc, 1, sizeof(gamepad_hid_report_desc), f);
        fclose(f);
    } else {
        return -1;
    }
    
    // Create configuration
    snprintf(path, sizeof(path), "%s/configs/c.1", gadget->gadget_path);
    mkdir(path, 0755);
    
    snprintf(path, sizeof(path), "%s/configs/c.1/strings/0x409", gadget->gadget_path);
    mkdir(path, 0755);
    
    snprintf(path, sizeof(path), "%s/configs/c.1/strings/0x409/configuration", gadget->gadget_path);
    write_file(path, "Gamepad Config");
    
    // Set configuration attributes
    snprintf(path, sizeof(path), "%s/configs/c.1/MaxPower", gadget->gadget_path);
    write_file(path, "250");
    
    // Link HID function to configuration
    snprintf(path, sizeof(path), "%s/configs/c.1/hid.usb0", gadget->gadget_path);
    symlink("../functions/hid.usb0", path);
    
    // Save HID path for later
    snprintf(gadget->hid_path, sizeof(gadget->hid_path),
             "%s/functions/hid.usb0/dev", gadget->gadget_path);
    
    gadget->initialized = true;
    
    return 0;
}

int usb_gadget_disable(usb_gadget_t *gadget) {
    char path[512];
    
    // Disable gadget
    snprintf(path, sizeof(path), "%s/UDC", gadget->gadget_path);
    write_file(path, "");
    
    // Remove links
    snprintf(path, sizeof(path), "%s/configs/c.1/hid.usb0", gadget->gadget_path);
    unlink(path);
    
    // Remove directories (simplified - in production you'd remove all)
    snprintf(path, sizeof(path), "%s/configs/c.1/strings/0x409", gadget->gadget_path);
    rmdir(path);
    
    snprintf(path, sizeof(path), "%s/configs/c.1", gadget->gadget_path);
    rmdir(path);
    
    snprintf(path, sizeof(path), "%s/functions/hid.usb0", gadget->gadget_path);
    rmdir(path);
    
    snprintf(path, sizeof(path), "%s/strings/0x409", gadget->gadget_path);
    rmdir(path);
    
    rmdir(gadget->gadget_path);
    
    gadget->initialized = false;
    gadget->connected = false;
    
    return 0;
}

int usb_gadget_send_report(usb_gadget_t *gadget, const uint8_t *report, size_t size) {
    if (!gadget->connected) {
        // Try to find HID device file
        char hid_dev_path[256];
        snprintf(hid_dev_path, sizeof(hid_dev_path), "/dev/hidg0");
        
        gadget->fd = open(hid_dev_path, O_RDWR | O_NONBLOCK);
        if (gadget->fd >= 0) {
            gadget->connected = true;
        } else {
            return -1;
        }
    }
    
    if (gadget->connected) {
        ssize_t ret = write(gadget->fd, report, size);
        if (ret < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                return 0;
            }
            close(gadget->fd);
            gadget->connected = false;
            return -1;
        }
        return ret;
    }
    
    return -1;
}

void usb_gadget_cleanup(usb_gadget_t *gadget) {
    if (gadget->connected && gadget->fd >= 0) {
        close(gadget->fd);
    }
    
    if (gadget->initialized) {
        usb_gadget_disable(gadget);
    }
    
    memset(gadget, 0, sizeof(usb_gadget_t));
}