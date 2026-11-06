#pragma once

#include <windows.h>

namespace mod::hooks {

// Verifies every hook site, then enables them all or none. Runs on the init thread.
bool install(HMODULE game);

} // namespace mod::hooks
