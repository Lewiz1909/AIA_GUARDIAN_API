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

void ConfigManager::printEvaluations() const {
    if (configData["scoring"]["pass_threshold"]) {
        std::cout << "Pass The Test" << std::endl;
    }
    else if (configData["scoring"]["warning_threshold"]) {
        std::cout << "Passed The Test, But Still Need Improvements" << std::endl;
    }
    else if (configData["scoring"]["failed_threshold"]) {
        std::cout << "Failed The Test" << std::endl;
    }
}

json ConfigManager::getRuleById(const std::string& id) const {
    auto rules_of_evaluation = configData["evaluation"]["criteria"];
    for (auto& assessment : rules_of_evaluation) {
        if (assessment["id"] = id) {
            std::cout << assessment["id"] << std::endl;
            std::cout << assessment["name"] << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            std::cout << assessment["rules"] << '\t';
            std::cout << std::endl;
            return assessment;
        }
    }
    return nullptr;
}


int main() {
    ConfigManager CM;
    std::mutex mtx;
    std::condition_variable cv;
    int state = 0;
    try  {
        auto Load_Config = [&]() {
            std::unique_lock<std::mutex> lock(mtx);
            if (CM.loadConfig("configs/default.json")) {
                std::cout << "[1] Config Successfully Loaded." << std::endl;
                state = 1;
            }
            else {
                std::cerr << "Failed To Load Configs" << std::endl;
            }
            cv.notify_all();
        };
        auto rulesReader = [&]() {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [&]() { return state >= 1; });
            std::cout << "[2] Fecthing Evaluation Rules" << std::endl;
            state = 2;
            CM.getRuleById("EVAL001");
            cv.notify_all();
        };
        auto printEval = [&]() {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [&]() { return state >= 2; });
            std::cout << "[3] Fecthing Evaluation Rules" << std::endl;
            state = 3;
            CM.printEvaluations();
            cv.notify_all();
        };
        auto saveConfigs = [&](){
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [&]() {return state >= 3;});
            if (CM.saveConfig("configs/default.json")) {
                std::cout << "[4] Saving Configs..." << std::endl;
                state = 4;
            }
            else {
                std::cout << "Failed To Save Configs ./Retry" << std::endl;
            }
        };
        std::thread t1(Load_Config);
        std::thread t2(rulesReader);
        std::thread t3(printEval);
        std::thread t4(saveConfigs);
        t1.join();
        t2.join();
        t3.join();
        t4.join();
    }
    catch(std::exception& e) {
        std::cout << e.what() << std::endl;
    }
    return 0;
}