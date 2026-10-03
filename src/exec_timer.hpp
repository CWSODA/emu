#include <chrono>
#include <iostream>

class ExecutionTimer {
   public:
    void start_timer() { start = std::chrono::high_resolution_clock::now(); }
    void stop_timer() {
        auto end = std::chrono::high_resolution_clock::now();
        float duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        std::cout << "Execution Timer: " << (end - start) << ", fps: " << 1.0 / (duration * 1e-6)
                  << '\n';
    }
    std::chrono::steady_clock::time_point start;
};