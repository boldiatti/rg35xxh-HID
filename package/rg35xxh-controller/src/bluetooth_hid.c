#include "bluetooth_hid.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <bluetooth/l2cap.h>
#include <bluetooth/hidp.h>

#define HID_CONN_SUCCESS 0
#define HID_CONN_FAILED -1

static int create_l2cap_sock(uint16_t psm) {
    struct sockaddr_l2 addr;
    int sock;
    
    sock = socket(PF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP);
    if (sock < 0) {
        perror("Failed to create L2CAP socket");
        return -1;
    }
    
    memset(&addr, 0, sizeof(addr));
    addr.l2_family = AF_BLUETOOTH;
    bacpy(&addr.l2_bdaddr, BDADDR_ANY);
    addr.l2_psm = htobs(psm);
    
    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("Failed to bind L2CAP socket");
        close(sock);
        return -1;
    }
    
    return sock;
}

int bluetooth_hid_init(bluetooth_hid_t *bt) {
    memset(bt, 0, sizeof(bluetooth_hid_t));
    
    // Initialize Bluetooth adapter
    int dev_id = hci_get_route(NULL);
    if (dev_id < 0) {
        dev_id = 0; // Use first adapter
    }
    
    int sock = hci_open_dev(dev_id);
    if (sock < 0) {
        fprintf(stderr, "Failed to open HCI socket\n");
        return -1;
    }
    
    // Get local adapter address
    struct hci_dev_info di;
    if (hci_devinfo(dev_id, &di) == 0) {
        ba2str(&di.bdaddr, bt->local_addr);
    }
    
    hci_close_dev(sock);
    
    // Create control socket
    bt->ctl_sock = socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_HCI);
    if (bt->ctl_sock < 0) {
        perror("Failed to create HCI control socket");
        return -1;
    }
    
    // Create L2CAP sockets for HID control and interrupt
    bt->sock = create_l2cap_sock(L2CAP_PSM_HIDP_CTRL);
    if (bt->sock < 0) {
        close(bt->ctl_sock);
        return -1;
    }
    
    bt->initialized = true;
    bt->discoverable = true;
    
    printf("Bluetooth HID initialized on %s\n", bt->local_addr);
    
    return 0;
}

int bluetooth_hid_start_advertising(bluetooth_hid_t *bt) {
    if (!bt->initialized) {
        return -1;
    }
    
    // Set class of device to Gamepad (0x002508)
    struct hci_request rq;
    uint8_t cls[3] = {0x08, 0x25, 0x00}; // Gamepad class
    
    memset(&rq, 0, sizeof(rq));
    rq.ogf = OGF_HOST_CTL;
    rq.ocf = OCF_WRITE_CLASS_OF_DEV;
    rq.cparam = 3;
    rq.clen = 3;
    rq.rparam = cls;
    
    if (hci_send_req(bt->ctl_sock, &rq, 1000) < 0) {
        perror("Failed to set device class");
        return -1;
    }
    
    // Set discoverable mode
    if (bt->discoverable) {
        struct hci_request rq2;
        uint8_t discoverable = 1;
        
        memset(&rq2, 0, sizeof(rq2));
        rq2.ogf = OGF_HOST_CTL;
        rq2.ocf = OCF_WRITE_SCAN_ENABLE;
        rq2.cparam = 1;
        rq2.clen = 1;
        rq2.rparam = &discoverable;
        
        if (hci_send_req(bt->ctl_sock, &rq2, 1000) < 0) {
            perror("Failed to set discoverable mode");
            return -1;
        }
    }
    
    printf("Bluetooth advertising started (class: Gamepad)\n");
    return 0;
}

int bluetooth_hid_stop_advertising(bluetooth_hid_t *bt) {
    if (!bt->initialized) {
        return -1;
    }
    
    // Disable scans
    struct hci_request rq;
    uint8_t disable = 0;
    
    memset(&rq, 0, sizeof(rq));
    rq.ogf = OGF_HOST_CTL;
    rq.ocf = OCF_WRITE_SCAN_ENABLE;
    rq.cparam = 1;
    rq.clen = 1;
    rq.rparam = &disable;
    
    if (hci_send_req(bt->ctl_sock, &rq, 1000) < 0) {
        perror("Failed to disable scans");
        return -1;
    }
    
    printf("Bluetooth advertising stopped\n");
    return 0;
}

int bluetooth_hid_set_discoverable(bluetooth_hid_t *bt, bool discoverable) {
    bt->discoverable = discoverable;
    
    if (discoverable) {
        return bluetooth_hid_start_advertising(bt);
    } else {
        return bluetooth_hid_stop_advertising(bt);
    }
}

int bluetooth_hid_send_report(bluetooth_hid_t *bt, const uint8_t *report, size_t size) {
    if (!bt->connected) {
        // Not connected to any device
        return -1;
    }
    
    // Send via interrupt channel
    // Implementation would need to maintain HID connection state
    // This is simplified - in production you'd use the BlueZ D-Bus API
    
    return 0;
}

void bluetooth_hid_cleanup(bluetooth_hid_t *bt) {
    if (bt->sock >= 0) {
        close(bt->sock);
    }
    if (bt->ctl_sock >= 0) {
        close(bt->ctl_sock);
    }
    memset(bt, 0, sizeof(bluetooth_hid_t));
}