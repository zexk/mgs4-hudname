// Adapted from mgs3-qcamo (MIT), itself from bbtracker (MIT).
#include "log.h"

#include <windows.h>

#include <cstdarg>
#include <cstdio>

namespace mod {
namespace {

FILE* g_file = nullptr;
bool g_verbose = false;
CRITICAL_SECTION g_lock;

const char* level_tag(LogLevel lvl)
{
    switch (lvl) {
    case LogLevel::Debug: return "DEBUG";
    case LogLevel::Info: return "INFO";
    case LogLevel::Warn: return "WARN";
    case LogLevel::Error: return "ERROR";
    }
    return "?????";
}

} // namespace

bool log_init(const char* path)
{
    if (g_file) {
        return true;
    }
    FILE* f = nullptr;
    if (fopen_s(&f, path, "w") != 0 || !f) {
        return false;
    }
    InitializeCriticalSection(&g_lock);
    g_file = f;
    return true;
}

void log_set_verbose(bool verbose)
{
    g_verbose = verbose;
}

void log_write(LogLevel lvl, const char* fmt, ...)
{
    if (!g_file || (lvl == LogLevel::Debug && !g_verbose)) {
        return;
    }
    SYSTEMTIME st{};
    GetLocalTime(&st);

    char msg[2048];
    va_list args;
    va_start(args, fmt);
    vsnprintf(msg, sizeof(msg), fmt, args);
    va_end(args);

    // Hooks log from game threads; keep lines whole.
    EnterCriticalSection(&g_lock);
    fprintf(g_file, "[%02u:%02u:%02u.%03u] [%s] %s\n", st.wHour, st.wMinute, st.wSecond,
            st.wMilliseconds, level_tag(lvl), msg);
    fflush(g_file);
    if (ferror(g_file)) {
        clearerr(g_file);
    }
    LeaveCriticalSection(&g_lock);
}

} // namespace mod
