#pragma once
#include <fstream>

inline std::ofstream& log() {
    static std::ofstream log("../logs/log.txt");
    return log;
}
#define LOG_LINE(s) log() << s << std::endl

inline std::ofstream& log_sp() {
    static std::ofstream log("../logs/stack_pointer_log.txt");
    return log;
}
#define LOG_SP_LINE(s)
// #define LOG_SP_LINE(s) log_sp() << std::hex << s << std::endl

inline std::ofstream& log_mem() {
    static std::ofstream log("../logs/mem_log.txt");
    return log;
}
#define LOG_MEM_LINE(s)
// #define LOG_MEM_LINE(s) log_sp() << std::hex << s << std::endl

inline std::ofstream& log_misc() {
    static std::ofstream log("../logs/misc_log.txt");
    return log;
}
// #define LOG_MISC_LINE(s)
#define LOG_MISC_LINE(s) log_misc() << std::hex << s << std::endl

inline std::ofstream& log_opcode() {
    static std::ofstream log("../logs/opcode_log.txt");
    return log;
}
// #define LOG_OPCODE_LINE(s)
#define LOG_OPCODE_LINE(s) log_opcode() << std::hex << s << std::endl