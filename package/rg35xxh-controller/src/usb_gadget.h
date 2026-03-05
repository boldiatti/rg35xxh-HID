#ifndef USB_GADGET_H
#define USB_GADGET_H

#include <stdint.h>
#include <stdbool.h>

// USB HID Gamepad Report Descriptor
#define GAMEPAD_HID_REPORT_DESC_SIZE 76

typedef struct {
    int fd;
    bool initialized;
    bool connected;
    char gadget_path[256];
    char hid_path[256];
} usb_gadget_t;

int usb_gadget_init(usb_gadget_t *gadget);
int usb_gadget_enable(usb_gadget_t *gadget);
int usb_gadget_disable(usb_gadget_t *gadget);
int usb_gadget_send_report(usb_gadget_t *gadget, const uint8_t *report, size_t size);
void usb_gadget_cleanup(usb_gadget_t *gadget);

#endif // USB_GADGET_H