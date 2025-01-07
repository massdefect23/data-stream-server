#ifndef DOTENV_H
#define DOTENV_H

#include <string>
#include <unordered_map>

class DotEnv
{
    public:
        bool load(const std::string& filename);
        std::string get(const std::string& key, const std::string& defaultValue = "") const;

    private:
        std::unordered_map<std::string, std::string> envMap_;
};


#endif // DOTENV_H