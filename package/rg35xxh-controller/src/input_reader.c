#include "input_reader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <errno.h>

static const char *INPUT_DEV_DIR = "/dev/input";
static const char *EVENT_PREFIX = "event";

static int is_gamepad_event(const char *name) {
    // Check if this is a gamepad input device
    return (strstr(name, "gpio") != NULL || 
            strstr(name, "adc") != NULL ||
            strstr(name, "joystick") != NULL ||
            strstr(name, "gamepad") != NULL);
}

int input_reader_init(input_reader_t *reader) {
    memset(reader, 0, sizeof(input_reader_t));
    return input_reader_scan_devices(reader);
}

int input_reader_scan_devices(input_reader_t *reader) {
    DIR *dir;
    struct dirent *entry;
    char path[256];
    int found = 0;
    
    dir = opendir(INPUT_DEV_DIR);
    if (!dir) {
        perror("Failed to open /dev/input");
        return -1;
    }
    
    while ((entry = readdir(dir)) != NULL && found < 8) {
        if (strncmp(entry->d_name, EVENT_PREFIX, strlen(EVENT_PREFIX)) != 0) {
            continue;
        }
        
        snprintf(path, sizeof(path), "%s/%s", INPUT_DEV_DIR, entry->d_name);
        
        int fd = open(path, O_RDWR | O_NONBLOCK);
        if (fd < 0) {
            continue;
        }
        
        // Get device name
        char name[256] = {0};
        if (ioctl(fd, EVIOCGNAME(sizeof(name)), name) < 0) {
            close(fd);
            continue;
        }
        
        // Get device ID
        struct input_id id;
        if (ioctl(fd, EVIOCGID, &id) < 0) {
            close(fd);
            continue;
        }
        
        // Check if this is a gamepad (has buttons/axes we care about)
        if (!is_gamepad_event(name)) {
            close(fd);
            continue;
        }
        
        printf("Found input device: %s (%s)\n", path, name);
        
        reader->devices[found].fd = fd;
        strncpy(reader->devices[found].name, name, sizeof(reader->devices[found].name) - 1);
        reader->devices[found].id = id;
        reader->devices[found].initialized = true;
        found++;
    }
    
    closedir(dir);
    reader->device_count = found;
    
    return found;
}

int input_reader_read_events(input_reader_t *reader, int timeout_ms) {
    struct pollfd fds[reader->device_count];
    int i;
    
    if (reader->device_count == 0) {
        return 0;
    }
    
    for (i = 0; i < reader->device_count; i++) {
        fds[i].fd = reader->devices[i].fd;
        fds[i].events = POLLIN;
        fds[i].revents = 0;
    }
    
    int ret = poll(fds, reader->device_count, timeout_ms);
    if (ret <= 0) {
        return ret;
    }
    
    for (i = 0; i < reader->device_count; i++) {
        if (fds[i].revents & POLLIN) {
            struct input_event ev;
            ssize_t size = read(fds[i].fd, &ev, sizeof(ev));
            
            if (size == sizeof(ev)) {
                if (ev.type == EV_KEY) {
                    if (reader->button_callback) {
                        reader->button_callback(ev.code, ev.value);
                    }
                } else if (ev.type == EV_ABS) {
                    if (reader->axis_callback) {
                        reader->axis_callback(ev.code, ev.value);
                    }
                }
            }
        }
    }
    
    return ret;
}

void input_reader_cleanup(input_reader_t *reader) {
    for (int i = 0; i < reader->device_count; i++) {
        if (reader->devices[i].fd >= 0) {
            close(reader->devices[i].fd);
        }
    }
    memset(reader, 0, sizeof(input_reader_t));
}