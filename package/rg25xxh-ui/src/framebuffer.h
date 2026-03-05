#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <stdint.h>

typedef struct {
    int fd;
    uint8_t *fbp;
    long screensize;
    int width;
    int height;
    int bpp;
} framebuffer_t;

int framebuffer_init(framebuffer_t *fb);
int framebuffer_clear(framebuffer_t *fb);
void framebuffer_cleanup(framebuffer_t *fb);

#endif // FRAMEBUFFER_H