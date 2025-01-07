#ifndef CONFIG_H
#define CONFIG_H

#include <string>

struct AppConfig {
    std::string     dbHost;
    int             dbPort;
    std::string     dbName;
    std::string     dbUser;
    std::string     dbPassword;

    // Multiple ports for different servers
    unsigned short marketDataPort;
    unsigned short momentumDataPort;
};

// Load config from .env
AppConfig loadConfig(const std::string& envFile = ".env");


#endif // CONFIG_H