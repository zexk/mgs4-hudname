// ASI entry point. Ultimate ASI Loader (winmm.dll/dinput8.dll proxy) loads us at process
// start; the work runs on a thread so DllMain stays loader-lock safe.

#include <windows.h>

#include <cstdint>
#include <string>

#include "config.h"
#include "hooks.h"
#include "log.h"
#include "mem.h"

namespace {

std::string module_dir()
{
    HMODULE self = nullptr;
    if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
                                | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                            reinterpret_cast<LPCWSTR>(&module_dir), &self)
        || !self) {
        return ".";
    }
    // No std::filesystem: path::string() fails to link statically on mingw GCC 16.
    char path[MAX_PATH]{};
    if (!GetModuleFileNameA(self, path, MAX_PATH)) {
        return ".";
    }
    const std::string full(path);
    return full.substr(0, full.find_last_of("\\/"));
}

DWORD WINAPI init_thread(LPVOID)
{
    const std::string dir = module_dir();
    mod::log_init((dir + "\\" MOD_NAME ".log").c_str());
    mod::config_load((dir + "\\" MOD_NAME ".ini").c_str());
    mod::log_set_verbose(mod::config().verbose);

    HMODULE game = GetModuleHandleW(nullptr);
    uint32_t stamp = 0;
    mod::mem::module_timestamp(game, stamp);
    LOG_INFO(MOD_NAME " " MOD_VERSION " starting (dir %s, game build 0x%08X)", dir.c_str(),
             stamp);

    if (!mod::config().enabled) {
        LOG_INFO("disabled by config");
        return 0;
    }
    mod::hooks::install(game);
    return 0;
}

} // namespace

BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID)
{
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(instance);
        if (HANDLE thread = CreateThread(nullptr, 0, init_thread, nullptr, 0, nullptr)) {
            CloseHandle(thread);
        }
    }
    return TRUE;
}
