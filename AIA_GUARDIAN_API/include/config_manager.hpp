#pragma once
#ifndef CONFIG_MANAGER
#define CONFIG_MANAGER
#include<nlohmann/json.hpp>
#include<iostream>
#include<string>
#include<mutex>
#include<condition_variable>
using json = nlohmann::json;
class ConfigManager {
    public:
    bool loadConfig(const std::string& filePath);
    bool saveConfig(const std::string& filePath) const;
    void printEvaluations() const;
    json getRuleById(const std::string& id) const;
    private:
    nlohmann::json configData;
};

#endif