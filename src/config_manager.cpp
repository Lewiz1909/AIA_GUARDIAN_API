#include"config_manager.hpp"
#include<thread>
#include<chrono>
#include<fstream>
bool ConfigManager::loadConfig(const std::string& file_path) {
    std::cout << "Loading from: " << file_path << std::endl;
    std::ifstream file(file_path);
    if (!file.is_open()) {
        return false;
    }
    file >> configData;
    
    return true;
}

bool ConfigManager::saveConfig(const std::string& save_location) const {
    std::ofstream file(save_location);
    if (!file.is_open()) {
        return false;
    }
    file << configData.dump(4);
    return true;
}

std::string ConfigManager::printEvaluations(int& score) const {
    if (score >= configData["evaluation"]["scoring"]["pass_threshold"] || (score < configData["evaluation"]["scoring"]["pass_threshold"] && score > configData["evaluation"]["scoring"]["warning_threshold"])) {
        return "Pass The Test";
    }
    else if (score <= configData["evaluation"]["scoring"]["warning_threshold"] && score > configData["evaluation"]["scoring"]["failed_threshold"]) {
        return "Passed The Test, But Still Need Improvements";
    }
    else if (score <= configData["evaluation"]["scoring"]["failed_threshold"]) {
        return "Failed The Test";
    }
    return " ";
}

json ConfigManager::getRuleById(const std::string& id, int& score) const {
    auto rules_of_evaluation = configData["evaluation"]["criteria"];
    for (auto& assessment : rules_of_evaluation) {
        if (assessment["id"] == id) {
            std::cout << assessment["id"] << std::endl;
            std::cout << assessment["name"] << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            std::cout << assessment["rules"] << '\t';
            std::cout << std::endl; 
            if (assessment["pass"] == true) {
                std::cout << "Passed Evalution: " << assessment["id"] << std::endl;
                score += assessment["weight"].get<double>()*100;
            }
            else {
                std::cout << "Failed Evaluation: " << assessment["id"] << std::endl;
            }
            return assessment;
        }
    }
    std::cout << score << std::endl;
    return json();
}
