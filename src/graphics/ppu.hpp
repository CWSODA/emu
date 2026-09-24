#pragma once
#include <stdlib.h>
#include "tile.hpp"
#include "../logger.hpp"
#include "memory_addr.hpp"

constexpr int TILE_COUNT = 384;

struct OAMObject {
    uint8_t y_pos, x_pos, tile_idx, attribs;
};

class PPU {
   public:
    PPU() {}
    void init(uint8_t* memory) { this->memory = memory; }
    void tick(uint8_t cycles);
    void set_config(uint8_t LCDC);
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
        memory[STAT_ADDR] = scanline_count;
        scan_OAM();
    }
    uint8_t LYC;

    void print_VRAM();
    void print_tiling(Tile* tiles, int width, int height, const char* path);
    void background();

   private:
    uint8_t* memory;
    uint16_t scanline_count = 0;
    bool y_cond = false;

    void scan_OAM();
    Tile get_tile(uint8_t idx);
};