// Hook sites. Each one is an RVA plus the stock bytes expected there; nothing is
// enabled unless every site matches, so a new game build degrades to a no-op with a
// log line instead of a crash.

#include "hooks.h"

#include "config.h"
#include "log.h"
#include "mem.h"

#include <MinHook.h>

#include <cstdint>
#include <cstring>
#include <iterator>

namespace mod::hooks {
namespace {

// Layout text setters, build 26451734 (PE 0x6AE07392), both (ctx, node, text).
// Each copies text into a buffer from ctx's allocator (ctx+0x20) and stores it at
// node+0x38. The layout loader (0x308EE3) calls 0x30FCA0, which also expands
// \\ \n \r \x escapes, for nodes flagged 0x1000000 (the HUD's "OLD SNAKE"), and
// 0x30F6B0, the plain copy, for the rest; menus call 0x30F6B0 at runtime.
constexpr uint8_t kSetText6B0[] = {0x48, 0x89, 0x5C, 0x24, 0x20, 0x56, 0x57,
                                   0x41, 0x57, 0x48, 0x83, 0xEC, 0x20};
constexpr uint8_t kSetTextCA0[] = {0x48, 0x89, 0x4C, 0x24, 0x08, 0x53, 0x55, 0x56,
                                   0x57, 0x41, 0x55, 0x48, 0x83, 0xEC, 0x30};

using set_text_fn = int (*)(void* ctx, uintptr_t node, const char* text);
set_text_fn g_orig_6b0 = nullptr;
set_text_fn g_orig_ca0 = nullptr;

const char* swap(const char* site, uintptr_t node, const char* text)
{
    if (!text) {
        return text;
    }
    if (std::strcmp(text, config().match.c_str()) == 0) {
        LOG_DEBUG("%s(%llx) '%s' -> '%s'", site, static_cast<unsigned long long>(node), text,
                  config().name.c_str());
        return config().name.c_str();
    }
    LOG_DEBUG("%s(%llx) '%s'", site, static_cast<unsigned long long>(node), text);
    return text;
}

int hook_6b0(void* ctx, uintptr_t node, const char* text)
{
    return g_orig_6b0(ctx, node, swap("6b0", node, text));
}
int hook_ca0(void* ctx, uintptr_t node, const char* text)
{
    return g_orig_ca0(ctx, node, swap("ca0", node, text));
}

struct Site {
    const char* name;
    uintptr_t rva;
    const uint8_t* bytes;
    size_t len;
    void* detour;
    void** orig;
};

const Site kSites[] = {
    {"set_text_6b0", 0x30F6B0, kSetText6B0, sizeof(kSetText6B0),
     reinterpret_cast<void*>(&hook_6b0), reinterpret_cast<void**>(&g_orig_6b0)},
    {"set_text_ca0", 0x30FCA0, kSetTextCA0, sizeof(kSetTextCA0),
     reinterpret_cast<void*>(&hook_ca0), reinterpret_cast<void**>(&g_orig_ca0)},
};

// .text is decrypted in place shortly after process start: poll, don't fail on first read.
bool wait_for_bytes(HMODULE game, const Site& site, DWORD timeout_ms)
{
    const ULONGLONG deadline = GetTickCount64() + timeout_ms;
    while (!mem::bytes_at(game, site.rva, site.bytes, site.len)) {
        if (GetTickCount64() >= deadline) {
            uint8_t found[32] = {};
            mem::copy(mem::at(game, site.rva), found, site.len);
            LOG_ERROR("%s: bytes at +0x%llX do not match (first %02X %02X %02X %02X)",
                      site.name, static_cast<unsigned long long>(site.rva), found[0],
                      found[1], found[2], found[3]);
            return false;
        }
        Sleep(50);
    }
    return true;
}

} // namespace

bool install(HMODULE game)
{
    for (const Site& site : kSites) {
        if (site.rva == 0) {
            LOG_WARN("%s: no RVA set, skeleton build, not hooking", site.name);
            return false;
        }
        if (!wait_for_bytes(game, site, 30000)) {
            return false;
        }
    }

    if (MH_Initialize() != MH_OK) {
        LOG_ERROR("MH_Initialize failed");
        return false;
    }
    for (const Site& site : kSites) {
        void* target = reinterpret_cast<void*>(mem::at(game, site.rva));
        if (const MH_STATUS st = MH_CreateHook(target, site.detour, site.orig); st != MH_OK) {
            LOG_ERROR("%s: MH_CreateHook failed: %s", site.name, MH_StatusToString(st));
            MH_Uninitialize();
            return false;
        }
    }
    if (const MH_STATUS st = MH_EnableHook(MH_ALL_HOOKS); st != MH_OK) {
        LOG_ERROR("MH_EnableHook failed: %s", MH_StatusToString(st));
        MH_Uninitialize();
        return false;
    }
    LOG_INFO("%u hook(s) enabled", static_cast<unsigned>(std::size(kSites)));
    return true;
}

} // namespace mod::hooks
