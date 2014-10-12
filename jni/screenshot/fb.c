/*
 *   -- http://android-fb2png.googlecode.com/svn/trunk/fb.c --
 *
 *   Copyright 2011, Kyan He <kyan.ql.he@gmail.com>
 *
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include "log.h"
#include "fb.h"
#include "img_process.h"

// log fb info
void fb_dump(const struct fb* fb)
{
    printf("%13s : %d\n", "bpp", fb->bpp);
    printf("%13s : %d\n", "size", fb->size);
    printf("%13s : %d\n", "width", fb->width);
    printf("%13s : %d\n", "height", fb->height);
    printf("%13s : %d %d %d %d\n", "ARGB offset",
            fb->alpha_offset, fb->red_offset,
            fb->green_offset, fb->blue_offset);
    printf("%13s : %d %d %d %d\n", "ARGB length",
            fb->alpha_length, fb->red_length,
            fb->green_length, fb->blue_length);
}

/**
 * Returns the format of fb.
 */
static int fb_get_format(const struct fb *fb)
{
    int ao = fb->alpha_offset;
    int ro = fb->red_offset;
    int go = fb->green_offset;
    int bo = fb->blue_offset;

#define FB_FORMAT_UNKNOWN   0
#define FB_FORMAT_RGB565    1
#define FB_FORMAT_ARGB8888  2
#define FB_FORMAT_RGBA8888  3
#define FB_FORMAT_ABGR8888  4
#define FB_FORMAT_BGRA8888  5
#define FB_FORMAT_RGBX8888  FB_FORMAT_RGBA8888

    /* TODO: use offset */
    if (fb->bpp == 16)
        return FB_FORMAT_RGB565;

    /* TODO: validate */
    if (ao == 0 && ro == 8)
        return FB_FORMAT_ARGB8888;

    if (ao == 0 && ro == 24 && go == 16 && bo == 8)
        return FB_FORMAT_RGBX8888;

    if (ao == 0 && bo == 8)
        return FB_FORMAT_ABGR8888;

    if (ro == 0)
        return FB_FORMAT_RGBA8888;

    if (bo == 0)
        return FB_FORMAT_BGRA8888;

    /* fallback */
    return FB_FORMAT_UNKNOWN;
}

int fb_save_png(const struct fb *fb, const char *path)
{
    char *rgb_matrix;
    int ret = -1;

    /* Allocate RGB Matrix. */
    rgb_matrix = malloc(fb->width * fb->height * 3);
    if(!rgb_matrix) {
        printf("rgb_matrix: memory error\n");
        return -1;
    }

    int fmt = fb_get_format(fb);
    printf("Framebuffer Pixel Format: %d\n", fmt);

    switch(fmt) {
        case FB_FORMAT_RGB565:
            /* emulator use rgb565 */
            ret = rgb565_to_rgb888(fb->data,
                    rgb_matrix, fb->width * fb->height);
            break;
        case FB_FORMAT_ARGB8888:
            /* most devices use argb8888 */
            ret = argb8888_to_rgb888(fb->data,
                    rgb_matrix, fb->width * fb->height);
            break;
        case FB_FORMAT_ABGR8888:
            ret = abgr8888_to_rgb888(fb->data,
                    rgb_matrix, fb->width * fb->height);
            break;
        case FB_FORMAT_BGRA8888:
            ret = bgra8888_to_rgb888(fb->data,
                    rgb_matrix, fb->width * fb->height);
            break;
        case FB_FORMAT_RGBA8888:
            ret = rgba8888_to_rgb888(fb->data,
                    rgb_matrix, fb->width * fb->height);
            break;
        default:
            printf("Unsupported framebuffer type.\n");
            break;
    }

    if (ret != 0) {
        printf("Error while processing input image.\n");
    } else {
        /* Save in PNG format. */
        ret = save_png(path, rgb_matrix, fb->width, fb->height);
        if (ret != 0)
            printf("Failed to save in PNG format.\n");
    }

    free(rgb_matrix);
    free(fb->data);
    return ret;
}
