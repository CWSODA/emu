#include <stdlib.h>

// 160x144
struct WindowData {
    uint8_t wy, wx;  // 0xff4a, 0xff4b
};

class OAMObject {
    uint8_t y_pos, x_pos, tile_idx, attribs;
};