#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <stdint.h>
#include <stdbool.h>
#include <linux/input.h>

// Button mappings
#define RG35XXH_BUTTON_COUNT 17
#define RG35XXH_AXIS_COUNT 6

typedef enum {
    RG35XXH_BTN_A = 0,
    RG35XXH_BTN_B,
    RG35XXH_BTN_X,
    RG35XXH_BTN_Y,
    RG35XXH_BTN_L1,
    RG35XXH_BTN_R1,
    RG35XXH_BTN_L2,
    RG35XXH_BTN_R2,
    RG35XXH_BTN_SELECT,
    RG35XXH_BTN_START,
    RG35XXH_BTN_L3,
    RG35XXH_BTN_R3,
    RG35XXH_BTN_UP,
    RG35XXH_BTN_DOWN,
    RG35XXH_BTN_LEFT,
    RG35XXH_BTN_RIGHT,
    RG35XXH_BTN_HOME
} rg35xxh_button_t;

typedef enum {
    RG35XXH_AXIS_LX = 0,
    RG35XXH_AXIS_LY,
    RG35XXH_AXIS_RX,
    RG35XXH_AXIS_RY,
    RG35XXH_AXIS_LT,
    RG35XXH_AXIS_RT
} rg35xxh_axis_t;

// HID report structure
typedef struct __attribute__((packed)) {
    uint8_t report_id;      // Always 0x01 for gamepad
    uint8_t buttons[3];      // 24 buttons total
    uint8_t hat_switch;      // D-pad as hat
    uint8_t lx;              // Left stick X
    uint8_t ly;              // Left stick Y
    uint8_t rx;              // Right stick X
    uint8_t ry;              // Right stick Y
    uint8_t vendor[2];       // Vendor-specific data
} hid_gamepad_report_t;

// Controller state
typedef struct {
    bool buttons[RG35XXH_BUTTON_COUNT];
    int16_t axes[RG35XXH_AXIS_COUNT];
    uint8_t battery_level;
    bool usb_connected;
    bool bt_connected;
    bool bt_pairing_mode;
    bool bt_discoverable;
} controller_state_t;

// Function prototypes
void controller_init(controller_state_t *state);
void controller_update_button(controller_state_t *state, uint16_t code, int value);
void controller_update_axis(controller_state_t *state, uint16_t code, int value);
void controller_build_hid_report(const controller_state_t *state, hid_gamepad_report_t *report);
void controller_reset(controller_state_t *state);
const char* controller_button_name(rg35xxh_button_t btn);

#endif // CONTROLLER_H