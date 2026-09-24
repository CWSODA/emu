#include "ppu.hpp"

void PPU::print_tiling(Tile* tiles, int width, int height, const char* path) {
    std::ofstream img(path, std::ios::binary);
    img << "P5\n" << (width * 8) << " " << (height * 8) << "\n255\n";

    // reorder tiles
    for (int y = 0; y < height; y++) {
        // for every 8 pixels vertically
        for (int ty = 0; ty < 8; ty++) {
            uint8_t line[width * 8];
            for (int x = 0; x < width; x++) {
                for (int tx = 0; tx < 8; tx++) {
                    line[x * 8 + tx] = tiles[y * width + x].at(tx, ty);
                }
            }
            img.write(reinterpret_cast<const char*>(line), sizeof(line));
        }
    }

    img.close();
}

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

constexpr int pixel_w = 16;
constexpr int pixel_h = 24;
void PPU::print_VRAM() {
    std::ofstream img_line("../logs/frame_line.pgm", std::ios::binary);
    img_line << "P5\n" << 8 << " " << (TILE_COUNT * 8) << "\n255\n";

    // parse tiles
    Tile tiles[pixel_w * pixel_h];
    int count = 0;
    for (int addr = VRAM_START; addr <= VRAM_END; addr += 16) {
        // 2 bytes per line, 2-bit color
        Tile tile;
        for (int y = 0; y < 8; y++) {
            uint8_t LSB = mem[addr + 2 * y];
            uint8_t MSB = mem[addr + 2 * y + 1];

            for (int x = 0; x < 8; x++) {
                uint8_t val = LSB & (1 << x) + 2 * (MSB & (1 << x));
                tile.at(x, y, true) = val * 50;
            }
        }
        img_line.write(reinterpret_cast<const char*>(tile.data), sizeof(tile.data));
        tiles[count++] = tile;
    }

    print_tiling(tiles, pixel_w, pixel_h, "../logs/tiled_frame.pgm");

    img_line.close();
    background();
}

// $9800-$9BFF and $9C00-$9FFF
// 32x32 tiles
// 256x256 pixels
constexpr uint16_t BG_START = 0x9800;
constexpr uint16_t BG_END = 0x9bff;
void PPU::background() {
    Tile tiles[1 + BG_END - BG_START];
    int count = 0;
    for (int addr = BG_START; addr <= BG_END; addr++) {
        tiles[count++] = get_tile(mem[addr]);
    }
    print_tiling(tiles, 32, 32, "../logs/bg.pgm");
}

Tile PPU::get_tile(uint8_t idx) {
    // 2 bytes per line, 2-bit color
    Tile tile;
    uint16_t addr = VRAM_START + idx * 16;
    for (int y = 0; y < 8; y++) {
        uint8_t LSB = mem[addr + 2 * y];
        uint8_t MSB = mem[addr + 2 * y + 1];

        for (int x = 0; x < 8; x++) {
            uint8_t val = LSB & (1 << x) + 2 * (MSB & (1 << x));
            tile.at(x, y, true) = val * 50;
        }
    }
    return tile;
}