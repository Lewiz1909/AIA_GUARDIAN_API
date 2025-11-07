#define _WIN32_WINNT 0x0A00
#include<nlohmann/json.hpp>
#include"config_manager.hpp"
#include<unordered_set>
#include<chrono>
#include<iostream>
#include<httplib.h>
#include<fstream>
#include<filesystem>
using json = nlohmann::json;
int main() {
    std::unordered_set<int> Submission; 
    std::unordered_set<int> Report;
    httplib::Server server;
    /* INITIALIZATION */
    /* GET */

    server.Get(R"(/(\d+)/reports)", [&](const httplib::Request& req, httplib::Response& res){
    int submission_id = std::stoi(req.matches[1]);
    std::string path = "data/" + std::to_string(submission_id) + ".json";
    std::ifstream f(path);
    if(!f.is_open()){
        res.status = 404;
        res.set_content("No reports for this submission id", "text/plain");
        return;
    }

    json submission;
    f >> submission;

    json response = {
        {"data", submission["reports"]}
    };

    res.set_content(response.dump(4), "application/json");
});

    /*POST*/

    server.Post(R"(/(\d+)/reports)", [&](const httplib::Request& req, httplib::Response& res){
        json body;
        try {
            body = json::parse(req.body);
        }
        catch (...) {
            res.status = 404;
            res.set_content("Invalid JSON", "text/plain");
            return;
        }
        int submission_id = std::stoi(req.matches[1]);
        int report_id = body["report_id"];
        if (!body.contains("report_id") || body["report_id"] == std::string::npos || !body.contains("link")) {
            res.status = 404;
            res.set_content("report_id + link required", "text/plain");
        } // update time
            auto now = std::chrono::system_clock::now();
            std::time_t t = std::chrono::system_clock::to_time_t(now);
            std::string date = std::ctime(&t);
        //
        std::string PATH = "data/" + std::to_string(submission_id) + ".json";
        json submission;
        std::ifstream file(PATH);
        if (!file.is_open()) {
            submission = {
            {"submission_id", submission_id},
            {"reports", json::array()}, 
            {"edited_at", json::array()}
            };
        }  
        else {
            file >> submission;
        }

        submission["reports"].push_back(body);
        submission["edited_at"].push_back(date);
        std::ofstream save_file(PATH);
        save_file << submission.dump(4);
        if (Submission.count(submission_id) || Report.count(report_id)) {
            res.status = 404;
            res.set_content("Submission Or Report_Id Already Existed!", "text/plain");
            return;
        }
        Submission.insert(submission_id);
        Report.insert(report_id);
        res.set_content(submission.dump(4), "application/json");
    });


    /*PATCH*/

    server.Patch(R"(/(\d+)/reports/(\d+))", [&](const httplib::Request& req, httplib::Response& res){
        json body = json::parse(req.body);
        // WHITELIST KEYWORDS //
        static const std::unordered_set<std::string> ALLOWED_EDITS = {
            "link",
            "status",
            "description",
            "credit"
        };
        /* KHONG CHO EDIT REPORT_ID */
        if (body.contains("report_id")) {
            res.status = 400;
            res.set_content("report_id is immutable", "text/plain");
            return;
        }

        /* Check Keywords */
        for(auto& item : body.items()) {
            const std::string& key = item.key();
            if (!ALLOWED_EDITS.count(key)) {
                res.status = 404;
                res.set_content("field " + key + "IS NOT ALLOWED TO EDIT", "text/plain");
                return;
            }
        }

        int submission_id = std::stoi(req.matches[1]);
        int report_id = std::stoi(req.matches[2]);
        std::string PATH = "data/" + std::to_string(submission_id) + ".json";
        json submission;
        std::ifstream file(PATH);
        if (!file.is_open()) {
            res.status = 404;
            res.set_content("File Not Found",  "text/plain");
            return;
        }
        file >> submission;
        bool found = false;
        auto& submission_reports = submission["reports"];
        for (auto& reports : submission_reports) {
            if (reports["report_id"] == report_id) {
                reports.merge_patch(body);
                found = true;
            }
        }
        if (!found) {
            res.status = 404;
            res.set_content("Invalid & Report_ID Not Existed", "text/plain");
            return;
        }
        std::ofstream output_file(PATH);
        output_file << submission.dump(4);
        res.set_content(submission.dump(4), "application/json");
    });

    /*DELETE*/
    server.Delete(R"(/(\d+)/reports/(\d+))", [&](const httplib::Request& req, httplib::Response& res){
        int submission_id = std::stoi(req.matches[1]);
        int report_id = std::stoi(req.matches[2]);
        std::string PATH = "data/" + std::to_string(submission_id) + ".json";

        /*Read File*/
        std::ifstream file(PATH);
        if (!file.is_open()) {
            res.status = 404;
            res.set_content("File NOT FOUND", "text/plain");
            return;
        }
        json submission;
        file >> submission;
        bool found = false;
        auto& Report_IDs = submission["reports"];
        for (size_t i = 0; i < Report_IDs.size(); i++) {
            if (Report_IDs[i]["report_id"] == report_id) {
                Report_IDs.erase(Report_IDs.begin() + i);
                found = true;
                break;
            }
        }
        if (!found) {
            res.status = 404;
            res.set_content("ReportID NOT FOUND", "text/plain");
            return;
        }

        std::ofstream Output_File(PATH);
        Output_File << submission.dump(4);
        res.set_content(submission.dump(4), "application/json");
    });


    server.Get("/reports", [&](const httplib::Request& req, httplib::Response& res){

        json out;
        out["data"] = json::array();  

        for (auto& file : std::filesystem::directory_iterator("data")) {
            if (file.path().extension() == ".json") {

                std::string stem = file.path().stem().string(); 
                try {
                    int id = std::stoi(stem);
                    out["data"].push_back(id);
                }
                catch(...) {}
            }
        }

        res.set_content(out.dump(4), "application/json");
    });
    
    /* LISTENING TO SERVERS */
        std::cout << "server on http://127.0.0.1:8000\n";
        server.listen("127.0.0.1", 8000);
    return 0;
}


