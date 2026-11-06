// Log helper in the house style: adapted from mgs3-qcamo (MIT), itself from bbtracker (MIT).
#pragma once

namespace mod {

enum class LogLevel : int {
    Debug = 0,
    Info,
    Warn,
    Error,
};

bool log_init(const char* path);
void log_set_verbose(bool verbose);
void log_write(LogLevel lvl, const char* fmt, ...)
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((format(printf, 2, 3)))
#endif
    ;

} // namespace mod

#define LOG_DEBUG(...) ::mod::log_write(::mod::LogLevel::Debug, __VA_ARGS__)
#define LOG_INFO(...) ::mod::log_write(::mod::LogLevel::Info, __VA_ARGS__)
#define LOG_WARN(...) ::mod::log_write(::mod::LogLevel::Warn, __VA_ARGS__)
#define LOG_ERROR(...) ::mod::log_write(::mod::LogLevel::Error, __VA_ARGS__)
