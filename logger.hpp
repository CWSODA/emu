#pragma once
#include <fstream>
#include <iostream>

// please ignore this horrible way of logging :>

#define ENABLE_CPU_DUMP false

#define ENABLE_LOG true
#define ENABLE_LOG_SP false
#define ENABLE_LOG_MEM false
#define ENABLE_LOG_MISC false
#define ENABLE_LOG_OPCODE false

inline std::ofstream& log() {
    static std::ofstream log("../logs/log.txt");
    return log;
}
#if ENABLE_LOG
#define LOG_LINE(s) log() << s << '\n'
#else
#define LOG_LINE(s)
#endif

inline std::ofstream& log_sp() {
    static std::ofstream log("../logs/stack_pointer_log.txt");
    return log;
}
#if ENABLE_LOG_SP
#define LOG_SP_LINE(s) log_sp() << std::hex << s << '\n'
#else
#define LOG_SP_LINE(s)
#endif

inline std::ofstream& log_mem() {
    static std::ofstream log("../logs/mem_log.txt");
    return log;
}
#if ENABLE_LOG_MEM
#define LOG_MEM_LINE(s) log_mem() << std::hex << s << '\n'
#else
#define LOG_MEM_LINE(s)
#endif

inline std::ofstream& log_misc() {
    static std::ofstream log("../logs/misc_log.txt");
    return log;
}
#if ENABLE_LOG_MISC
#define LOG_MISC_LINE(s) log_misc() << std::hex << s << '\n'
#else
#define LOG_MISC_LINE(s)
#endif

inline std::ofstream& log_opcode() {
    static std::ofstream log("../logs/opcode_log.txt");
    return log;
}
#if ENABLE_LOG_OPCODE
#define LOG_OPCODE_LINE(s) log_opcode() << std::hex << s << '\n'
#else
#define LOG_OPCODE_LINE(s)
#endif