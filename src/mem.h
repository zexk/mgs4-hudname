// Memory access in the house style (bbtracker).
//
// Reads go through ReadProcessMemory on our own process rather than a raw pointer
// dereference, so a stale or wrong address fails a check instead of taking the game down.
// Fixed offsets are RVAs into a module whose .text may be decrypted at runtime, so
// anything can be wrong on a build we have not seen.
#pragma once

#include <windows.h>

#include <cstddef>
#include <cstdint>
#include <cstring>

namespace mod::mem {

inline bool copy(uintptr_t address, void* out, size_t size)
{
    SIZE_T copied = 0;
    return ReadProcessMemory(GetCurrentProcess(), reinterpret_cast<const void*>(address), out,
                             size, &copied)
        && copied == size;
}

template <typename T>
bool copy(uintptr_t address, T& out)
{
    return copy(address, &out, sizeof(out));
}

// base + rva, the form every fixed offset is written in.
inline uintptr_t at(HMODULE module, uintptr_t rva)
{
    return reinterpret_cast<uintptr_t>(module) + rva;
}

// Does the module have these exact bytes at `rva`?
//
// The one check a hook needs before it may touch the module. Master Collection .text is
// encrypted at rest and decrypted in place before the real entry point runs, so the code
// at any RVA is either the instruction stream we expect or still ciphertext. Both look
// like "some bytes", and the second would get us a hook on garbage.
inline bool bytes_at(HMODULE module, uintptr_t rva, const uint8_t* expect, size_t len)
{
    uint8_t found[32] = {};
    if (len > sizeof(found) || !copy(at(module, rva), found, len)) {
        return false;
    }
    return std::memcmp(found, expect, len) == 0;
}

// PE header TimeDateStamp: identifies the game build in the log and gates fixed RVAs.
inline bool module_timestamp(HMODULE module, uint32_t& out)
{
    const uintptr_t base = reinterpret_cast<uintptr_t>(module);
    IMAGE_DOS_HEADER dos{};
    if (!base || !copy(base, dos) || dos.e_magic != IMAGE_DOS_SIGNATURE
        || dos.e_lfanew < 0 || dos.e_lfanew > 0x1000000) {
        return false;
    }
    IMAGE_NT_HEADERS nt{};
    if (!copy(base + static_cast<uintptr_t>(dos.e_lfanew), nt)
        || nt.Signature != IMAGE_NT_SIGNATURE) {
        return false;
    }
    out = nt.FileHeader.TimeDateStamp;
    return true;
}

} // namespace mod::mem
