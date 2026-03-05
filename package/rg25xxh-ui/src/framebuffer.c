#include "framebuffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>

int framebuffer_init(framebuffer_t *fb) {
    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;
    
    memset(fb, 0, sizeof(framebuffer_t));
    
    // Open framebuffer device
    fb->fd = open("/dev/fb0", O_RDWR);
    if (fb->fd < 0) {
        perror("Failed to open framebuffer");
        return -1;
    }
    
    // Get fixed screen info
    if (ioctl(fb->fd, FBIOGET_FSCREENINFO, &finfo) < 0) {
        perror("Failed to get fixed screen info");
        close(fb->fd);
        return -1;
    }
    
    // Get variable screen info
    if (ioctl(fb->fd, FBIOGET_VSCREENINFO, &vinfo) < 0) {
        perror("Failed to get variable screen info");
        close(fb->fd);
        return -1;
    }
    
    fb->width = vinfo.xres;
    fb->height = vinfo.yres;
    fb->bpp = vinfo.bits_per_pixel;
    fb->screensize = finfo.smem_len;
    
    // Map framebuffer
    fb->fbp = mmap(0, fb->screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fb->fd, 0);
    if (fb->fbp == MAP_FAILED) {
        perror("Failed to map framebuffer");
        close(fb->fd);
        return -1;
    }
    
    printf("Framebuffer: %dx%d, %d bpp\n", fb->width, fb->height, fb->bpp);
    
    return 0;
}

int framebuffer_clear(framebuffer_t *fb) {
    if (fb->fbp) {
        memset(fb->fbp, 0, fb->screensize);
        return 0;
    }
    return -1;
}

void framebuffer_cleanup(framebuffer_t *fb) {
    if (fb->fbp) {
        munmap(fb->fbp, fb->screensize);
    }
    if (fb->fd >= 0) {
        close(fb->fd);
    }
    memset(fb, 0, sizeof(framebuffer_t));
}