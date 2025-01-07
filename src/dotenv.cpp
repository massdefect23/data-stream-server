#include "dotenv.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>

bool DotEnv::load(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        // Trim whitespace
        line.erase(std::remove_if(line.begin(), line.end(), ::isspace), line.end());
        // Skip empty or commented lines
        if (line.empty() || line[0] == '#') continue;

        // Split into key=value
        auto pos = line.find('=');
        if (pos == std::string::npos) continue;

        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);

        envMap_[key] = value;
    }

    return true;
}

std::string DotEnv::get(const std::string& key, const std::string& defaultValue) const {
    auto it = envMap_.find(key);
    if (it != envMap_.end()) {
        return it->second;
    }
    return defaultValue;
}