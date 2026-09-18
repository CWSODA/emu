#include <fstream>

#define LOG_LINE(s) log() << s << std::endl

inline std::ofstream& log() {
    static std::ofstream log("log.txt");
    return log;
}