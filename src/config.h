#pragma once

#include <string>

namespace mod {

struct Config {
    bool enabled = true;
    bool verbose = false;
    std::string match = "OLD SNAKE";
    std::string name = "GOLD SNAKE";
};

const Config& config();
void config_load(const char* path);

} // namespace mod
