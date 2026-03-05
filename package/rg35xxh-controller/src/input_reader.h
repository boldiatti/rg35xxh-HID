#ifndef INPUT_READER_H
#define INPUT_READER_H

#include <stdbool.h>
#include <linux/input.h>

typedef struct {
    int fd;
    char name[256];
    struct input_id id;
    bool initialized;
} input_device_t;

typedef struct {
    input_device_t devices[8];
    int device_count;
    void (*button_callback)(uint16_t code, int value);
    void (*axis_callback)(uint16_t code, int value);
} input_reader_t;

int input_reader_init(input_reader_t *reader);
int input_reader_scan_devices(input_reader_t *reader);
int input_reader_read_events(input_reader_t *reader, int timeout_ms);
void input_reader_cleanup(input_reader_t *reader);

#endif // INPUT_READER_H