#pragma once
#include <stdlib.h>
#include "tile.hpp"
#include "logger.hpp"
#include "memory_addr.hpp"
#include "color.hpp"

constexpr int TILE_COUNT = 384;
// constexpr int SCREEN_WIDTH = 160;
// constexpr int SCREEN_HEIGHT = 144;
constexpr int SCREEN_WIDTH = 32 * 8;
constexpr int SCREEN_HEIGHT = 32 * 8;

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
    void init(uint8_t* memory);
    void tick(uint8_t cycles);
    void set_config(uint8_t LCDC);
    void set_lcd_stat(uint8_t lcd_stat);
    uint8_t LYC;

    bool check_frame_ready() {
        if (!is_frame_ready) return false;
        is_frame_ready = false;  // reset if frame ready
        return true;
    }

    const uint8_t* get_frame_data() { return reinterpret_cast<uint8_t*>(frame); }
    void draw_background();

    void print_VRAM();
    void print_tiling(Tile* tiles, int width, int height, const char* path);
    void print_background();

   private:
    uint8_t* memory;
    uint16_t scanline = 0;
    uint32_t dots = 0;
    bool y_cond = false;

    // frame data
    ColorRGBA frame[SCREEN_WIDTH * SCREEN_HEIGHT];
    bool is_frame_ready = false;  // flag for when frame has been updated

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