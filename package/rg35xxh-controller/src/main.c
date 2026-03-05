#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <time.h>
#include <sys/timerfd.h>

#include "controller.h"
#include "input_reader.h"
#include "usb_gadget.h"
#include "bluetooth_hid.h"

static volatile int running = 1;
static controller_state_t controller_state;
static input_reader_t input_reader;
static usb_gadget_t usb_gadget;
static bluetooth_hid_t bluetooth_hid;

static void signal_handler(int sig) {
    running = 0;
}

static void button_event_callback(uint16_t code, int value) {
    controller_update_button(&controller_state, code, value);
}

static void axis_event_callback(uint16_t code, int value) {
    controller_update_axis(&controller_state, code, value);
}

static void* usb_thread_func(void* arg) {
    hid_gamepad_report_t report;
    
    while (running) {
        if (controller_state.usb_connected) {
            controller_build_hid_report(&controller_state, &report);
            usb_gadget_send_report(&usb_gadget, (uint8_t*)&report, sizeof(report));
        }
        usleep(4000); // 4ms = 250Hz update rate
    }
    
    return NULL;
}

static void* bluetooth_thread_func(void* arg) {
    hid_gamepad_report_t report;
    
    while (running) {
        if (controller_state.bt_connected) {
            controller_build_hid_report(&controller_state, &report);
            bluetooth_hid_send_report(&bluetooth_hid, (uint8_t*)&report, sizeof(report));
        }
        usleep(4000); // 4ms = 250Hz update rate
    }
    
    return NULL;
}

static void check_combinations(void) {
    static bool prev_pairing = false;
    static bool prev_usb_toggle = false;
    
    // USB toggle combo (SELECT + START)
    if (controller_state.buttons[RG35XXH_BTN_SELECT] && 
        controller_state.buttons[RG35XXH_BTN_START] && !prev_usb_toggle) {
        controller_state.usb_connected = !controller_state.usb_connected;
        if (controller_state.usb_connected) {
            usb_gadget_enable(&usb_gadget);
            printf("USB gadget enabled\n");
        } else {
            usb_gadget_disable(&usb_gadget);
            printf("USB gadget disabled\n");
        }
    }
    prev_usb_toggle = controller_state.buttons[RG35XXH_BTN_SELECT] && 
                     controller_state.buttons[RG35XXH_BTN_START];
    
    // Bluetooth pairing combo (SELECT + L1)
    if (controller_state.buttons[RG35XXH_BTN_SELECT] && 
        controller_state.buttons[RG35XXH_BTN_L1] && !prev_pairing) {
        controller_state.bt_pairing_mode = true;
        bluetooth_hid_set_discoverable(&bluetooth_hid, true);
        printf("Bluetooth pairing mode enabled\n");
    }
    prev_pairing = controller_state.buttons[RG35XXH_BTN_SELECT] && 
                  controller_state.buttons[RG35XXH_BTN_L1];
}

int main(int argc, char **argv) {
    printf("RG35XXH Controller Daemon v1.0\n");
    printf("================================\n");
    
    // Set up signal handling
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    // Initialize components
    controller_init(&controller_state);
    
    input_reader.button_callback = button_event_callback;
    input_reader.axis_callback = axis_event_callback;
    
    if (input_reader_init(&input_reader) < 0) {
        fprintf(stderr, "Failed to initialize input reader\n");
        return 1;
    }
    
    printf("Found %d input devices\n", input_reader.device_count);
    
    if (usb_gadget_init(&usb_gadget) < 0) {
        fprintf(stderr, "Failed to initialize USB gadget\n");
    } else {
        printf("USB gadget initialized\n");
        // Enable USB by default
        controller_state.usb_connected = true;
        usb_gadget_enable(&usb_gadget);
    }
    
    if (bluetooth_hid_init(&bluetooth_hid) < 0) {
        fprintf(stderr, "Failed to initialize Bluetooth HID\n");
    } else {
        printf("Bluetooth HID initialized\n");
        bluetooth_hid_start_advertising(&bluetooth_hid);
        controller_state.bt_connected = true;
    }
    
    // Create threads
    pthread_t usb_thread, bt_thread;
    pthread_create(&usb_thread, NULL, usb_thread_func, NULL);
    pthread_create(&bt_thread, NULL, bluetooth_thread_func, NULL);
    
    // Main event loop
    printf("Entering main event loop...\n");
    
    while (running) {
        input_reader_read_events(&input_reader, 100); // 100ms timeout
        
        check_combinations();
    }
    
    printf("Shutting down...\n");
    
    // Clean up
    running = 0;
    pthread_join(usb_thread, NULL);
    pthread_join(bt_thread, NULL);
    
    input_reader_cleanup(&input_reader);
    usb_gadget_cleanup(&usb_gadget);
    bluetooth_hid_cleanup(&bluetooth_hid);
    
    printf("Goodbye!\n");
    
    return 0;
}