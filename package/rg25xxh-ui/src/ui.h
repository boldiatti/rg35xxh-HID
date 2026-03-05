#ifndef UI_H
#define UI_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    bool buttons[17];
    int16_t axes[6];
    uint8_t battery;
    bool usb_connected;
    bool bt_connected;
    bool bt_pairing;
    bool bt_discoverable;
} ui_state_t;

typedef struct {
    void *framebuffer;
    int width;
    int height;
    int bpp;
} display_t;

int ui_init(display_t *display);
int ui_update(display_t *display, const ui_state_t *state);
void ui_cleanup(display_t *display);

#endif // UI_H