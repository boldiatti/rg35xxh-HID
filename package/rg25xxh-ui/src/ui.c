#include "ui.h"
#include "framebuffer.h"
#include <stdio.h>
#include <string.h>
#include <SDL2/SDL.h>

#define BUTTON_SIZE 40
#define BUTTON_SPACING 10
#define MARGIN 20

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static SDL_Texture *texture = NULL;
static TTF_Font *font = NULL;

static const char* button_names[] = {
    "A", "B", "X", "Y", "L1", "R1", "L2", "R2",
    "SEL", "STA", "L3", "R3", "UP", "DN", "LT", "RT", "HOM"
};

static void draw_button(SDL_Renderer *renderer, int x, int y, const char *label, bool pressed) {
    SDL_Rect rect = {x, y, BUTTON_SIZE, BUTTON_SIZE};
    
    // Draw button background
    if (pressed) {
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); // Green when pressed
    } else {
        SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255); // Gray when not pressed
    }
    SDL_RenderFillRect(renderer, &rect);
    
    // Draw border
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &rect);
    
    // Draw label (simplified - in production use TTF)
    // For now, just use filled rect with text position
}

static void draw_battery(SDL_Renderer *renderer, int x, int y, int level) {
    SDL_Rect outline = {x, y, 60, 30};
    SDL_Rect fill = {x + 2, y + 2, (56 * level) / 100, 26};
    
    // Battery outline
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &outline);
    
    // Battery tip
    SDL_Rect tip = {x + 60, y + 10, 5, 10};
    SDL_RenderFillRect(renderer, &tip);
    
    // Battery level
    if (level > 20) {
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); // Green
    } else {
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Red
    }
    SDL_RenderFillRect(renderer, &fill);
}

static void draw_status_icon(SDL_Renderer *renderer, int x, int y, bool active, const char *label) {
    SDL_Rect rect = {x, y, 40, 40};
    
    if (active) {
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    } else {
        SDL_SetRenderDrawColor(renderer, 64, 64, 64, 255);
    }
    SDL_RenderFillRect(renderer, &rect);
    
    // Draw text placeholder
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
}

static void draw_analog_stick(SDL_Renderer *renderer, int x, int y, int16_t x_val, int16_t y_val) {
    SDL_Rect bg = {x, y, 100, 100};
    
    // Background
    SDL_SetRenderDrawColor(renderer, 32, 32, 32, 255);
    SDL_RenderFillRect(renderer, &bg);
    
    // Cross lines
    SDL_SetRenderDrawColor(renderer, 64, 64, 64, 255);
    SDL_RenderDrawLine(renderer, x + 50, y, x + 50, y + 100);
    SDL_RenderDrawLine(renderer, x, y + 50, x + 100, y + 50);
    
    // Stick position
    int stick_x = x + 50 + ((x_val * 40) / 32768);
    int stick_y = y + 50 + ((y_val * 40) / 32768);
    
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_Rect stick = {stick_x - 5, stick_y - 5, 10, 10};
    SDL_RenderFillRect(renderer, &stick);
}

int ui_init(display_t *display) {
    // Initialize SDL with framebuffer
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL initialization failed: %s\n", SDL_GetError());
        return -1;
    }
    
    // Create window using framebuffer
    window = SDL_CreateWindow("RG35XXH Controller",
                              SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED,
                              display->width, display->height,
                              SDL_WINDOW_FULLSCREEN_DESKTOP);
    
    if (!window) {
        fprintf(stderr, "Window creation failed: %s\n", SDL_GetError());
        SDL_Quit();
        return -1;
    }
    
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    if (!renderer) {
        fprintf(stderr, "Renderer creation failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }
    
    // Hide cursor
    SDL_ShowCursor(SDL_DISABLE);
    
    return 0;
}

int ui_update(display_t *display, const ui_state_t *state) {
    // Clear screen
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    // Draw title
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    // In production, use TTF for text
    
    // Draw battery
    draw_battery(renderer, display->width - 100, 10, state->battery);
    
    // Draw USB status
    draw_status_icon(renderer, display->width - 160, 10, state->usb_connected, "USB");
    
    // Draw Bluetooth status
    draw_status_icon(renderer, display->width - 220, 10, state->bt_connected, "BT");
    if (state->bt_pairing) {
        // Show pairing indicator
        SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
        SDL_Rect pairing = {display->width - 220, 60, 40, 5};
        SDL_RenderFillRect(renderer, &pairing);
    }
    
    // Draw buttons
    int start_x = MARGIN;
    int start_y = MARGIN + 50;
    
    // First row: ABXY
    draw_button(renderer, start_x, start_y, "A", state->buttons[0]);
    draw_button(renderer, start_x + BUTTON_SIZE + BUTTON_SPACING, start_y, "B", state->buttons[1]);
    draw_button(renderer, start_x + 2*(BUTTON_SIZE + BUTTON_SPACING), start_y, "X", state->buttons[2]);
    draw_button(renderer, start_x + 3*(BUTTON_SIZE + BUTTON_SPACING), start_y, "Y", state->buttons[3]);
    
    // Second row: Shoulder buttons
    start_y += BUTTON_SIZE + BUTTON_SPACING;
    draw_button(renderer, start_x, start_y, "L1", state->buttons[4]);
    draw_button(renderer, start_x + BUTTON_SIZE + BUTTON_SPACING, start_y, "R1", state->buttons[5]);
    draw_button(renderer, start_x + 2*(BUTTON_SIZE + BUTTON_SPACING), start_y, "L2", state->buttons[6]);
    draw_button(renderer, start_x + 3*(BUTTON_SIZE + BUTTON_SPACING), start_y, "R2", state->buttons[7]);
    
    // Third row: Select, Start, Home
    start_y += BUTTON_SIZE + BUTTON_SPACING;
    draw_button(renderer, start_x, start_y, "SEL", state->buttons[8]);
    draw_button(renderer, start_x + BUTTON_SIZE + BUTTON_SPACING, start_y, "STA", state->buttons[9]);
    draw_button(renderer, start_x + 2*(BUTTON_SIZE + BUTTON_SPACING), start_y, "L3", state->buttons[10]);
    draw_button(renderer, start_x + 3*(BUTTON_SIZE + BUTTON_SPACING), start_y, "R3", state->buttons[11]);
    
    // D-pad at bottom left
    start_y += BUTTON_SIZE + BUTTON_SPACING;
    draw_button(renderer, start_x + BUTTON_SIZE, start_y, "UP", state->buttons[12]);
    draw_button(renderer, start_x, start_y + BUTTON_SIZE + BUTTON_SPACING, "LT", state->buttons[14]);
    draw_button(renderer, start_x + BUTTON_SIZE, start_y + BUTTON_SIZE + BUTTON_SPACING, "DN", state->buttons[13]);
    draw_button(renderer, start_x + 2*BUTTON_SIZE + BUTTON_SPACING, start_y + BUTTON_SIZE + BUTTON_SPACING, "RT", state->buttons[15]);
    
    // Draw analog sticks
    draw_analog_stick(renderer, display->width - 250, display->height - 150, 
                     state->axes[0], state->axes[1]); // Left stick
    
    draw_analog_stick(renderer, display->width - 120, display->height - 150,
                     state->axes[2], state->axes[3]); // Right stick
    
    // Present renderer
    SDL_RenderPresent(renderer);
    
    return 0;
}

void ui_cleanup(display_t *display) {
    if (renderer) {
        SDL_DestroyRenderer(renderer);
    }
    if (window) {
        SDL_DestroyWindow(window);
    }
    SDL_Quit();
}