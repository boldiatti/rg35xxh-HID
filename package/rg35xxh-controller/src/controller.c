#include "controller.h"
#include <string.h>
#include <stdio.h>

// Linux input event codes to RG35XXH button mapping
typedef struct {
    uint16_t ev_code;
    rg35xxh_button_t btn;
    bool is_axis;
} input_mapping_t;

static const input_mapping_t button_map[] = {
    { BTN_SOUTH,     RG35XXH_BTN_A,      false },
    { BTN_EAST,      RG35XXH_BTN_B,      false },
    { BTN_NORTH,     RG35XXH_BTN_X,      false },
    { BTN_WEST,      RG35XXH_BTN_Y,      false },
    { BTN_TL,        RG35XXH_BTN_L1,     false },
    { BTN_TR,        RG35XXH_BTN_R1,     false },
    { BTN_TL2,       RG35XXH_BTN_L2,     false },
    { BTN_TR2,       RG35XXH_BTN_R2,     false },
    { BTN_SELECT,    RG35XXH_BTN_SELECT, false },
    { BTN_START,     RG35XXH_BTN_START,  false },
    { BTN_THUMBL,    RG35XXH_BTN_L3,     false },
    { BTN_THUMBR,    RG35XXH_BTN_R3,     false },
    { BTN_DPAD_UP,   RG35XXH_BTN_UP,     false },
    { BTN_DPAD_DOWN, RG35XXH_BTN_DOWN,   false },
    { BTN_DPAD_LEFT, RG35XXH_BTN_LEFT,   false },
    { BTN_DPAD_RIGHT,RG35XXH_BTN_RIGHT,  false },
    { BTN_MODE,      RG35XXH_BTN_HOME,   false },
    { 0, 0, false }
};

// Axis mappings
static const input_mapping_t axis_map[] = {
    { ABS_X,        RG35XXH_AXIS_LX, true },
    { ABS_Y,        RG35XXH_AXIS_LY, true },
    { ABS_RX,       RG35XXH_AXIS_RX, true },
    { ABS_RY,       RG35XXH_AXIS_RY, true },
    { ABS_Z,        RG35XXH_AXIS_LT, true },
    { ABS_RZ,       RG35XXH_AXIS_RT, true },
    { 0, 0, false }
};

void controller_init(controller_state_t *state) {
    memset(state, 0, sizeof(controller_state_t));
    state->battery_level = 100;
    state->bt_discoverable = true;
    
    // Initialize axes to center positions
    for (int i = 0; i < RG35XXH_AXIS_COUNT; i++) {
        if (i == RG35XXH_AXIS_LT || i == RG35XXH_AXIS_RT) {
            state->axes[i] = 0; // Triggers start at 0
        } else {
            state->axes[i] = 0; // Sticks will be centered (0 = 128 in HID space)
        }
    }
}

void controller_update_button(controller_state_t *state, uint16_t code, int value) {
    for (int i = 0; button_map[i].ev_code != 0; i++) {
        if (button_map[i].ev_code == code) {
            state->buttons[button_map[i].btn] = (value != 0);
            break;
        }
    }
    
    // Check for special button combinations
    if (state->buttons[RG35XXH_BTN_SELECT] && state->buttons[RG35XXH_BTN_START]) {
        // Toggle USB mode
        state->usb_connected = !state->usb_connected;
    }
    
    if (state->buttons[RG35XXH_BTN_SELECT] && state->buttons[RG35XXH_BTN_L1]) {
        // Enter Bluetooth pairing mode
        state->bt_pairing_mode = true;
        state->bt_discoverable = true;
    }
    
    if (state->buttons[RG35XXH_BTN_SELECT] && state->buttons[RG35XXH_BTN_R1]) {
        // Toggle Bluetooth visibility
        state->bt_discoverable = !state->bt_discoverable;
    }
}

void controller_update_axis(controller_state_t *state, uint16_t code, int value) {
    for (int i = 0; axis_map[i].ev_code != 0; i++) {
        if (axis_map[i].ev_code == code) {
            state->axes[axis_map[i].btn] = value;
            break;
        }
    }
}

void controller_build_hid_report(const controller_state_t *state, hid_gamepad_report_t *report) {
    memset(report, 0, sizeof(hid_gamepad_report_t));
    
    report->report_id = 0x01;
    
    // Map buttons to HID button bits (24 buttons max)
    // Button mapping: A=1, B=2, X=3, Y=4, L1=5, R1=6, L2=7, R2=8, SELECT=9, START=10, L3=11, R3=12, HOME=13
    
    if (state->buttons[RG35XXH_BTN_A])      report->buttons[0] |= 0x01;
    if (state->buttons[RG35XXH_BTN_B])      report->buttons[0] |= 0x02;
    if (state->buttons[RG35XXH_BTN_X])      report->buttons[0] |= 0x04;
    if (state->buttons[RG35XXH_BTN_Y])      report->buttons[0] |= 0x08;
    if (state->buttons[RG35XXH_BTN_L1])     report->buttons[0] |= 0x10;
    if (state->buttons[RG35XXH_BTN_R1])     report->buttons[0] |= 0x20;
    if (state->buttons[RG35XXH_BTN_L2])     report->buttons[0] |= 0x40;
    if (state->buttons[RG35XXH_BTN_R2])     report->buttons[0] |= 0x80;
    
    if (state->buttons[RG35XXH_BTN_SELECT]) report->buttons[1] |= 0x01;
    if (state->buttons[RG35XXH_BTN_START])  report->buttons[1] |= 0x02;
    if (state->buttons[RG35XXH_BTN_L3])     report->buttons[1] |= 0x04;
    if (state->buttons[RG35XXH_BTN_R3])     report->buttons[1] |= 0x08;
    if (state->buttons[RG35XXH_BTN_HOME])   report->buttons[1] |= 0x10;
    
    // Map D-pad as hat switch (0-7, 8 = neutral)
    if (state->buttons[RG35XXH_BTN_UP] && !state->buttons[RG35XXH_BTN_DOWN]) {
        if (state->buttons[RG35XXH_BTN_LEFT])
            report->hat_switch = 7; // Up-Left
        else if (state->buttons[RG35XXH_BTN_RIGHT])
            report->hat_switch = 1; // Up-Right
        else
            report->hat_switch = 0; // Up
    } else if (state->buttons[RG35XXH_BTN_DOWN] && !state->buttons[RG35XXH_BTN_UP]) {
        if (state->buttons[RG35XXH_BTN_LEFT])
            report->hat_switch = 5; // Down-Left
        else if (state->buttons[RG35XXH_BTN_RIGHT])
            report->hat_switch = 3; // Down-Right
        else
            report->hat_switch = 4; // Down
    } else if (state->buttons[RG35XXH_BTN_LEFT]) {
        report->hat_switch = 6; // Left
    } else if (state->buttons[RG35XXH_BTN_RIGHT]) {
        report->hat_switch = 2; // Right
    } else {
        report->hat_switch = 8; // Neutral
    }
    
    // Map analog sticks (convert from -32768..32767 to 0..255)
    report->lx = (state->axes[RG35XXH_AXIS_LX] + 32768) >> 8;
    report->ly = (state->axes[RG35XXH_AXIS_LY] + 32768) >> 8;
    report->rx = (state->axes[RG35XXH_AXIS_RX] + 32768) >> 8;
    report->ry = (state->axes[RG35XXH_AXIS_RY] + 32768) >> 8;
    
    // Map triggers (0..1023 to 0..255)
    report->vendor[0] = state->axes[RG35XXH_AXIS_LT] >> 2;
    report->vendor[1] = state->axes[RG35XXH_AXIS_RT] >> 2;
}

void controller_reset(controller_state_t *state) {
    memset(state->buttons, 0, sizeof(state->buttons));
    for (int i = 0; i < RG35XXH_AXIS_COUNT; i++) {
        if (i == RG35XXH_AXIS_LT || i == RG35XXH_AXIS_RT) {
            state->axes[i] = 0;
        } else {
            state->axes[i] = 0;
        }
    }
}

const char* controller_button_name(rg35xxh_button_t btn) {
    static const char* names[] = {
        "A", "B", "X", "Y", "L1", "R1", "L2", "R2", 
        "SELECT", "START", "L3", "R3", "UP", "DOWN", 
        "LEFT", "RIGHT", "HOME"
    };
    
    if (btn >= 0 && btn < RG35XXH_BUTTON_COUNT) {
        return names[btn];
    }
    return "UNKNOWN";
}