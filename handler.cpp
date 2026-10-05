#include <string>
#include <cstdio>
#include <cctype>
#include <cstdlib>
#include "handler.h"

static bool starts_with(const std::string& s, const char* p)
{
    size_t n = std::strlen(p);
    return s.size() >= n && std::memcmp(s.data(), p, n) == 0;
}

static bool find_str(const std::string& s, const char* key, size_t& pos)
{
    pos = s.find(key);
    return pos != std::string::npos;
}

static bool extract_bool(const std::string& s, const char* key, bool& out)
{
    size_t pos;
    if (!find_str(s, key, pos)) return false;
    pos = s.find(':', pos);
    if (pos == std::string::npos) return false;
    pos++;
    while (pos < s.size() && std::isspace((unsigned char)s[pos])) pos++;
    if (pos >= s.size()) return false;
    if (s.compare(pos, 4, "true") == 0) { out = true; return true; }
    if (s.compare(pos, 5, "false") == 0) { out = false; return true; }
    return false;
}

static bool extract_number(const std::string& s, const char* key, double& out)
{
    size_t pos;
    if (!find_str(s, key, pos)) return false;
    pos = s.find(':', pos);
    if (pos == std::string::npos) return false;
    pos++;
    char* end = nullptr;
    out = std::strtod(s.c_str() + pos, &end);
    return end != s.c_str() + pos;
}

static bool extract_string(const std::string& s, const char* key, std::string& out)
{
    size_t pos;
    if (!find_str(s, key, pos)) return false;
    pos = s.find(':', pos);
    if (pos == std::string::npos) return false;
    pos++;
    while (pos < s.size() && std::isspace((unsigned char)s[pos])) pos++;
    if (pos >= s.size() || s[pos] != '"') return false;
    pos++;
    size_t end = s.find('"', pos);
    if (end == std::string::npos) return false;
    out.assign(s.data() + pos, end - pos);
    return true;
}

void handle_message(const std::string& json)
{
    std::printf("[KronosPipe] recv: %s\n", json.c_str());

    std::string type;
    if (!extract_string(json, "\"type\"", type)) return;

    if (type == "feature")
    {
        std::string category, feature;
        bool enabled = false;
        if (!extract_string(json, "\"category\"", category)) return;
        if (!extract_string(json, "\"feature\"", feature)) return;
        if (!extract_bool(json, "\"enabled\"", enabled)) return;

        // All cheat message handling removed - ESP only mode
        return;
    }

    if (type == "property")
    {
        std::string category, feature, name;
        double value = 0.0;
        if (!extract_string(json, "\"category\"", category)) return;
        if (!extract_string(json, "\"feature\"", feature)) return;
        if (!extract_string(json, "\"name\"", name)) return;
        if (!extract_number(json, "\"value\"", value)) return;

        // All cheat property handling removed - ESP only mode
        return;
    }
}


