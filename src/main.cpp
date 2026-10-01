#include "gameboy.hpp"
#include "logger.hpp"
#include "render.hpp"

int main(int argc, char* argv[]) {
    SDLApp app;
    // app.init();

    GameBoy gb;
    if (argc == 2) {  // loads rom if argument given
        gb.load_rom_from_path(argv[1]);
    } else if (argc == 3) {  // loads test rom
        int test_rom_idx = std::stoi(argv[2]);
        printf("Loading test rom(%d)...\n", test_rom_idx);
        gb.load_test_rom(test_rom_idx);
    }
    gb.dump_memory();

    // start running
    while (true) {
        gb.run();
    }
    while (true) {
        if (!app.poll_input()) break;
        gb.run_n_cycles(100);
        if (gb.check_frame_ready()) {
            app.update_framebuffer(gb.get_frame_data());
        }
    }

    return 0;
}