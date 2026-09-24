#include <stdlib.h>

struct Tile {
    uint8_t data[8 * 8];  // 2-bit color, 8x8 pixels, store as 255 greyscale

    uint8_t& at(int x, int y, bool rev = false) {
        if (rev) x = 7 - x;  // reverse
        return data[y * 8 + x];
    }
};
