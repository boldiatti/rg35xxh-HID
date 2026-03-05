#ifndef BLUETOOTH_HID_H
#define BLUETOOTH_HID_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    int sock;
    int ctl_sock;
    bool initialized;
    bool connected;
    bool pairing_mode;
    bool discoverable;
    char local_addr[18];
    char remote_addr[18];
} bluetooth_hid_t;

int bluetooth_hid_init(bluetooth_hid_t *bt);
int bluetooth_hid_start_advertising(bluetooth_hid_t *bt);
int bluetooth_hid_stop_advertising(bluetooth_hid_t *bt);
int bluetooth_hid_set_discoverable(bluetooth_hid_t *bt, bool discoverable);
int bluetooth_hid_send_report(bluetooth_hid_t *bt, const uint8_t *report, size_t size);
void bluetooth_hid_cleanup(bluetooth_hid_t *bt);

#endif // BLUETOOTH_HID_H