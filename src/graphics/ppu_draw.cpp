#include "ppu.hpp"

constexpr uint16_t BG_START = 0x9800;
constexpr uint16_t BG_END = 0x9bff;

constexpr int BG_WIDTH = 32;
constexpr int BG_HEIGHT = 32;

void copy_cvt(uint8_t* from, ColorRGBA* to, size_t n) {
    for (int idx = 0; idx < n; idx++) {
        to[idx] = ColorRGBA::from_grey(from[idx]);
    }
}

// draw 32x32 background = 256 tiles
// optimize later so tiles arent read twice
void PPU::draw_background() {
    Tile tiles[1 + BG_END - BG_START];
    int count = 0;
    for (int addr = BG_START; addr <= BG_END; addr++) {
        tiles[count++] = get_tile(memory[addr]);
    }

    // reorder tiles
    size_t frame_idx = 0;
    for (int y = 0; y < BG_HEIGHT; y++) {
        // for every 8 pixels vertically
        for (int ty = 0; ty < 8; ty++) {
            uint8_t line[BG_WIDTH * 8];
            for (int x = 0; x < BG_WIDTH; x++) {
                for (int tx = 0; tx < 8; tx++) {
                    line[x * 8 + tx] = tiles[y * BG_WIDTH + x].at(tx, ty);
                }
            }
            // convert line over to framebuffer
            copy_cvt(line, &frame[frame_idx], sizeof(line));
            frame_idx += sizeof(line);
        }
    }
}