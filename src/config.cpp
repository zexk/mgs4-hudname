#include "config.h"

#include "log.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <string>

namespace mod {
namespace {

Config g_config;

std::string trim(const std::string& s)
{
    const auto begin = s.find_first_not_of(" \t\r\n");
    if (begin == std::string::npos) {
        return {};
    }
    const auto end = s.find_last_not_of(" \t\r\n");
    return s.substr(begin, end - begin + 1);
}

bool truthy(std::string v)
{
    std::transform(v.begin(), v.end(), v.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return v == "1" || v == "true" || v == "yes" || v == "on";
}

// The loader's setter expands backslash escapes and skips a fixed width after
// \x without checking for the terminator, so a backslash could make it read
// past the string. The HUD font covers printable ASCII; the life bar fits
// about 15 characters.
constexpr size_t kMaxText = 32;

bool valid_text(const char* key, const std::string& v)
{
    if (v.empty() || v.size() > kMaxText) {
        LOG_ERROR("config: %s must be 1-%u characters, got %u", key,
                  static_cast<unsigned>(kMaxText), static_cast<unsigned>(v.size()));
        return false;
    }
    for (const unsigned char c : v) {
        if (c < 0x20 || c > 0x7E || c == '\\') {
            LOG_ERROR("config: %s has unsupported character 0x%02X (printable ASCII, no "
                      "backslash)", key, c);
            return false;
        }
    }
    return true;
}

} // namespace

const Config& config()
{
    return g_config;
}

void config_load(const char* path)
{
    FILE* file = nullptr;
    if (fopen_s(&file, path, "r") != 0 || !file) {
        LOG_INFO("config %s not found, using defaults", path);
        return;
    }
    char buf[512];
    while (fgets(buf, sizeof(buf), file)) {
        const std::string line = trim(buf);
        if (line.empty() || line[0] == ';' || line[0] == '#') {
            continue;
        }
        const auto eq = line.find('=');
        if (eq == std::string::npos) {
            continue;
        }
        const std::string key = trim(line.substr(0, eq));
        const std::string value = trim(line.substr(eq + 1));
        if (key == "enabled") {
            g_config.enabled = truthy(value);
        } else if (key == "verbose") {
            g_config.verbose = truthy(value);
        } else if (key == "match") {
            g_config.match = value;
        } else if (key == "name") {
            g_config.name = value;
        } else {
            LOG_WARN("config: unknown key '%s'", key.c_str());
        }
    }
    fclose(file);
    if (!valid_text("match", g_config.match) || !valid_text("name", g_config.name)) {
        g_config.enabled = false;
    } else if (g_config.name.size() > 15) {
        LOG_WARN("config: name is %u characters, may overflow the life bar",
                 static_cast<unsigned>(g_config.name.size()));
    }
    LOG_INFO("config loaded: enabled=%d verbose=%d match='%s' name='%s'", g_config.enabled,
             g_config.verbose, g_config.match.c_str(), g_config.name.c_str());
}

} // namespace mod
