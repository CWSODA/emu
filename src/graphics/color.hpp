#include <stdint.h>
#include <stdio.h>

struct ColorRGBA {
    uint8_t r, g, b, a;
    ColorRGBA() {}
    ColorRGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
        this->r = r;
        this->g = g;
        this->b = b;
        this->a = a;
    }
    static ColorRGBA from_RGB(uint8_t r, uint8_t g, uint8_t b) { return ColorRGBA{r, g, b, 255}; }
    static ColorRGBA from_grey(uint8_t grey) { return ColorRGBA{grey, grey, grey, 255}; }

    void disp() { printf("Color RGBA: %d, %d, %d, %d\n", r, g, b, a); }
};