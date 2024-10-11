#ifndef _BITMAP_HELPERS_H_
#define _BITMAP_HELPERS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// adapted from https://stackoverflow.com/a/2654860/107642
// used for debugging
int create_bitmap_file(const char *filename, float *buffer, size_t w, size_t h) {
    int filesize = 54 + 3 * w * h;

    uint8_t *img = (uint8_t*)calloc(3 * w * h, 1);
    if (!img) {
        return -1; /* OOM */
    }
    for (size_t i = 0; i < w; i++) {
        for (size_t j = 0; j < h; j++) {
            float pixel_f = buffer[(j * w) + i];
            uint32_t pixel = static_cast<uint32_t>(pixel_f);
            uint8_t r = static_cast<uint8_t>(pixel >> 16 & 0xff);
            uint8_t g = static_cast<uint8_t>(pixel >> 8 & 0xff);
            uint8_t b = static_cast<uint8_t>(pixel & 0xff);

            size_t x = i;
            size_t y = (h - 1) - j;
            img[(x+y*w)*3+2] = (unsigned char)(r);
            img[(x+y*w)*3+1] = (unsigned char)(g);
            img[(x+y*w)*3+0] = (unsigned char)(b);
        }
    }

    uint8_t bmpfileheader[14] = { 'B','M', 0,0,0,0, 0,0, 0,0, 54,0,0,0 };
    uint8_t bmpinfoheader[40] = { 40,0,0,0, 0,0,0,0, 0,0,0,0, 1,0, 24,0 };
    uint8_t bmppad[3] = {0,0,0};

    bmpfileheader[ 2] = (uint8_t)(filesize    );
    bmpfileheader[ 3] = (uint8_t)(filesize>> 8);
    bmpfileheader[ 4] = (uint8_t)(filesize>>16);
    bmpfileheader[ 5] = (uint8_t)(filesize>>24);

    bmpinfoheader[ 4] = (uint8_t)(       w    );
    bmpinfoheader[ 5] = (uint8_t)(       w>> 8);
    bmpinfoheader[ 6] = (uint8_t)(       w>>16);
    bmpinfoheader[ 7] = (uint8_t)(       w>>24);
    bmpinfoheader[ 8] = (uint8_t)(       (-h)    );
    bmpinfoheader[ 9] = (uint8_t)(       (-h)>> 8);
    bmpinfoheader[10] = (uint8_t)(       (-h)>>16);
    bmpinfoheader[11] = (uint8_t)(       (-h)>>24);

    FILE *f = fopen(filename, "wb");
    if (!f) {
        free(img);
        return -2; /* could not open file */
    }
    fwrite(bmpfileheader, 1, 14, f);
    fwrite(bmpinfoheader, 1, 40, f);
    for(int i = 0; i < h; i++) {
        fwrite(img + (w * (h - i - 1) * 3), 3, w, f);
        fwrite(bmppad, 1, (4 - (w * 3) % 4 ) % 4, f);
    }
    free(img);
    fclose(f);

    return 0;
}

static uint32_t clamp(uint32_t value, uint32_t min, uint32_t max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

static uint32_t getPixel(float* src, int width, int height, int x, int y) {
    x = clamp(x, 0, width - 1);
    y = clamp(y, 0, height - 1);
    return (uint32_t)src[y * width + x];
}

static uint8_t interpolateChannel(uint8_t c00, uint8_t c10, uint8_t c01, uint8_t c11, float tx, float ty) {
    float a = c00 * (1 - tx) + c10 * tx;
    float b = c01 * (1 - tx) + c11 * tx;
    return (uint8_t)(a * (1 - ty) + b * ty);
}

static uint32_t interpolateColor(uint32_t c00, uint32_t c10, uint32_t c01, uint32_t c11, float tx, float ty) {
    uint8_t r00 = (c00 >> 16) & 0xFF;
    uint8_t g00 = (c00 >> 8) & 0xFF;
    uint8_t b00 = c00 & 0xFF;

    uint8_t r10 = (c10 >> 16) & 0xFF;
    uint8_t g10 = (c10 >> 8) & 0xFF;
    uint8_t b10 = c10 & 0xFF;

    uint8_t r01 = (c01 >> 16) & 0xFF;
    uint8_t g01 = (c01 >> 8) & 0xFF;
    uint8_t b01 = c01 & 0xFF;

    uint8_t r11 = (c11 >> 16) & 0xFF;
    uint8_t g11 = (c11 >> 8) & 0xFF;
    uint8_t b11 = c11 & 0xFF;

    uint8_t r = interpolateChannel(r00, r10, r01, r11, tx, ty);
    uint8_t g = interpolateChannel(g00, g10, g01, g11, tx, ty);
    uint8_t b = interpolateChannel(b00, b10, b01, b11, tx, ty);

    return (r << 16) | (g << 8) | b;
}

void resize_image(float* src, float* dst, int srcWidth, int srcHeight, int newWidth, int newHeight) {
    for (int y = 0; y < newHeight; y++) {
        for (int x = 0; x < newWidth; x++) {
            float gx = (float)(x * (srcWidth - 1)) / (newWidth - 1);
            float gy = (float)(y * (srcHeight - 1)) / (newHeight - 1);
            int gxi = (int)gx;
            int gyi = (int)gy;
            float tx = gx - gxi;
            float ty = gy - gyi;

            uint32_t c00 = getPixel(src, srcWidth, srcHeight, gxi, gyi);
            uint32_t c10 = getPixel(src, srcWidth, srcHeight, gxi + 1, gyi);
            uint32_t c01 = getPixel(src, srcWidth, srcHeight, gxi, gyi + 1);
            uint32_t c11 = getPixel(src, srcWidth, srcHeight, gxi + 1, gyi + 1);

            dst[y * newWidth + x] = interpolateColor(c00, c10, c01, c11, tx, ty);
        }
    }
}

#endif // _BITMAP_HELPERS_H_
