#pragma once
#include <stdlib.h>
#include "tile.hpp"
#include "../logger.hpp"
#include "memory_addr.hpp"

constexpr int TILE_COUNT = 384;

struct OAMObject {
    uint8_t y_pos, x_pos, tile_idx, attribs;
};

enum PPUState {
    OAM_SCAN = 2,
    SEND_PIXEL = 3,
    H_BLANK = 0,
    V_BLANK = 1,
};

class PPU {
   public:
    PPU() {}
    void init(uint8_t* memory) { this->memory = memory; }
    void tick(uint8_t cycles);
    void set_config(uint8_t LCDC);
    void set_lcd_stat(uint8_t lcd_stat);
    uint8_t LYC;

    void print_VRAM();
    void print_tiling(Tile* tiles, int width, int height, const char* path);
    void print_background();

   private:
    uint8_t* memory;
    uint16_t scanline = 0;
    bool y_cond = false;
    uint32_t dots = 0;

    PPUState ppu_state = V_BLANK;
    void set_ppu_state(PPUState state) {
        ppu_state = state;
        memory[LCD_STAT_ADDR] |= state;
    }

    OAMObject scanline_objs[10];  // can render max of 10
    uint8_t scanline_obj_n = 0;
    void scan_OAM();

    Tile get_tile(uint8_t idx);
};