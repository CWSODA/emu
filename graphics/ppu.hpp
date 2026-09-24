#pragma once
#include <stdlib.h>
#include "tile.hpp"
#include "../logger.hpp"

constexpr uint16_t OAM_START = 0xfe00;
constexpr uint16_t OAM_END = 0xfe9f;
constexpr uint16_t LCDC_ADDR = 0xff40;

constexpr uint16_t VRAM_START = 0x8000;
constexpr uint16_t VRAM_END = 0x97ff;
constexpr int TILE_COUNT = 384;

// STAT reg
constexpr uint16_t STAT_ADDR = 0xff44;

struct OAMObject {
    uint8_t y_pos, x_pos, tile_idx, attribs;
};

class PPU {
   public:
    PPU(uint8_t* mem) : mem(mem) {}
    void start_frame() {
        y_cond = false;
        scanline_count = 0;
        while (scanline_count < 154) {
            start_scanline();
            scanline_count++;
        }
    }
    void start_scanline() {
        // 456 dots per line
        mem[STAT_ADDR] = scanline_count;
        scan_OAM();
    }
    uint8_t LYC;

    void print_VRAM();
    void print_tiling(Tile* tiles, int width, int height, const char* path);

   private:
    uint8_t* mem;
    uint16_t scanline_count = 0;
    bool y_cond = false;

    void scan_OAM();
    void background();
    Tile get_tile(uint8_t idx);
};