#include <fstream>
#include <cstdint>
#include <vector>

class ROM {
   public:
    ROM(const char* filename) {
        std::ifstream file(filename, std::ios::binary);
        if (!file) throw std::runtime_error("Cannot read file!");

        data = std::vector<uint8_t>(std::istreambuf_iterator<char>(file), {});
    }

    std::vector<uint8_t> data;
};