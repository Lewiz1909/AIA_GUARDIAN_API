#pragma once
#ifndef CONFIG_MANAGER
#define CONFIG_MANAGER
#include<nlohmann/json.hpp>
#include<iostream>
#include<string>
using json = nlohmann::json;
class ConfigManager {
    public:
    bool loadConfig(const std::string& filePath);
    bool saveConfig(const std::string& filePath) const;
    std::string printEvaluations(int& score) const;
    json getRuleById(const std::string& id, int& score) const;
    json getConfigJSON() const {return configData;}
    private:
    nlohmann::json configData;
};

#endif