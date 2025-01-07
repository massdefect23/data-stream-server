#include "config.h"
#include "dotenv.h"

#include <mach-o/dyld.h>    // For _NSGetExecutablePath on macOS
#include <unistd.h>         // For PATH_MAX
#include <limits.h>         // For PATH_MAX
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

//-------------------------------------------------------------------
// Get the directory of the running executable (macOS only)
//-------------------------------------------------------------------
static std::string getExeDir() {
    char buffer[PATH_MAX];
    uint32_t size = sizeof(buffer);

    // _NSGetExecutablePath puts the actual path into 'buffer'
    if (_NSGetExecutablePath(buffer, &size) != 0) {
        std::cerr << "[getExeDir] _NSGetExecutablePath FAILED. Needed size: "
                  << size << std::endl;
        return {};
    }

    fs::path exePath = fs::weakly_canonical(fs::path(buffer));
    std::string exeDir = exePath.parent_path().string();

    std::cerr << "[getExeDir] raw exe path: " << buffer << std::endl;
    std::cerr << "[getExeDir] canonical exe path: " << exePath << std::endl;
    std::cerr << "[getExeDir] exeDir: " << exeDir << std::endl;

    return exeDir;
}

//-------------------------------------------------------------------
// Load config from .env located in the same folder as the executable
//-------------------------------------------------------------------
AppConfig loadConfig(const std::string& envFile /* optional */) {
    AppConfig config;
    DotEnv dotenv;

    // 1) If user passed an envFile, use that. 
    //    (Though typically you won't use it, but let's keep the parameter.)
    if (!envFile.empty()) {
        std::cerr << "[loadConfig] Using user-provided envFile: " << envFile << std::endl;
        if (!dotenv.load(envFile)) {
            std::cerr << "[loadConfig] WARNING: Could not load .env at: "
                      << envFile << std::endl;
            return config;
        }
    } else {
        // 2) Otherwise, get the directory of the running exe
        std::string exeDir = getExeDir();
        if (exeDir.empty()) {
            std::cerr << "[loadConfig] ERROR: Could not determine executable directory.\n";
            return config; // or throw an exception
        }

        // 3) Build path to .env
        fs::path envPath = fs::path(exeDir) / ".env";
        std::cerr << "[loadConfig] Attempting to load .env from: " << envPath << std::endl;

        // 4) Load
        if (!dotenv.load(envPath.string())) {
            std::cerr << "[loadConfig] WARNING: Could not load .env at: "
                      << envPath.string() << std::endl;
            return config;
        }
    }

    // 5) If we get here, .env is loaded. Parse variables:
    config.dbHost            = dotenv.get("DB_HOST");
    config.dbPort            = std::stoi(dotenv.get("DB_PORT"));
    config.dbName            = dotenv.get("DB_NAME");
    config.dbUser            = dotenv.get("DB_USER");
    config.dbPassword        = dotenv.get("DB_PASSWORD");
    config.marketDataPort    = static_cast<unsigned short>(std::stoi(
        dotenv.get("MARKET_DATA_PORT")));
    config.momentumDataPort  = static_cast<unsigned short>(std::stoi(
        dotenv.get("MOMENTUM_DATA_PORT")));

    return config;
}