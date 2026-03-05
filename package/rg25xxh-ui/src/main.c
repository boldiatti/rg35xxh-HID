#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "ui.h"
#include "framebuffer.h"

#define SHM_NAME "/rg35xxh_controller_state"

static volatile int running = 1;

static void signal_handler(int sig) {
    running = 0;
}

int main(int argc, char **argv) {
    printf("RG35XXH UI v1.0\n");
    
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    // Open shared memory for controller state
    int shm_fd = shm_open(SHM_NAME, O_RDONLY, 0666);
    if (shm_fd < 0) {
        // Try to create if not exists (controller might start later)
        shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
        if (shm_fd < 0) {
            perror("Failed to open shared memory");
            return 1;
        }
        ftruncate(shm_fd, sizeof(ui_state_t));
    }
    
    ui_state_t *shared_state = mmap(0, sizeof(ui_state_t), 
                                     PROT_READ, MAP_SHARED, shm_fd, 0);
    if (shared_state == MAP_FAILED) {
        perror("Failed to map shared memory");
        close(shm_fd);
        return 1;
    }
    
    // Initialize display
    display_t display;
    display.width = 640;
    display.height = 480;
    display.bpp = 32;
    
    if (ui_init(&display) < 0) {
        fprintf(stderr, "Failed to initialize UI\n");
        munmap(shared_state, sizeof(ui_state_t));
        close(shm_fd);
        return 1;
    }
    
    // Main loop
    ui_state_t local_state;
    
    while (running) {
        // Copy shared state
        memcpy(&local_state, shared_state, sizeof(ui_state_t));
        
        // Update display
        ui_update(&display, &local_state);
        
        usleep(16666); // 60fps
    }
    
    // Clean up
    ui_cleanup(&display);
    munmap(shared_state, sizeof(ui_state_t));
    close(shm_fd);
    shm_unlink(SHM_NAME);
    
    printf("UI shutdown complete\n");
    
    return 0;
}