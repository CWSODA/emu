#include <stdlib.h>
#include "../logger.hpp"

constexpr uint16_t OAM_START = 0xfe00;
constexpr uint16_t OAM_END = 0xfe9f;
constexpr uint16_t LCDC_ADDR = 0xff40;

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

   private:
    uint8_t* mem;
    uint16_t scanline_count = 0;
    bool y_cond = false;

    void scan_OAM();
};

void PPU::scan_OAM() {  // 80 dots
    // check LCDC
    uint8_t LCDC = mem[LCDC_ADDR];
    bool is_16 = LCDC & 0b100;

    // scan all 40 objects
    for (int idx = 0; idx < 40; idx += 4) {
        uint8_t y_pos = mem[OAM_START + idx];
        uint8_t x_pos = mem[OAM_START + idx + 1];
        uint8_t tile_idx = mem[OAM_START + idx + 2];
        uint8_t attribs = mem[OAM_START + idx + 3];

        // check if within scanline
        int y_min = scanline_count + 16;
        int y_max = scanline_count + 16 + (8 * is_16);
        if ((y_pos >= y_min) && (y_pos <= y_max)) {
            // within
            std::cout << "Obj " << idx << '\n';
        }
    }
}