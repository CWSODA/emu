#include <stdlib.h>

#include "data.hpp"

struct Tile {
    uint8_t data[2 * 8 * 8 / 8];  // 2-bit color, 8x8 pixels
};

// 0x8000 to 0x97ff
// 384 tiles
void parse_tile_addr(Data& data, uint16_t addr) {
    uint16_t addr = 0x8000;
    ;
}