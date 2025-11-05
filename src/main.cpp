#define _WIN32_WINNT 0x0A00
#include<nlohmann/json.hpp>
#include"config_manager.hpp"
#include<unordered_set>
#include<ctime>
#include<iostream>
#include<httplib.h>
#include<fstream>
using json = nlohmann::json;
int main() {
        std::unordered_set<int> Submission;
        std::unordered_set<int> Report;
    httplib::Server server;
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
        }
        std::string PATH = "data/" + std::to_string(submission_id) + ".json";
        json submission;
        std::ifstream file(PATH);
        if (!file.is_open()) {
            submission = {
            {"submission_id", submission_id},
            {"reports", json::array()}
            };
        }  
        else {
            file >> submission;
        }

        submission["reports"].push_back(body);

        std::ofstream save_file(PATH);
        save_file << submission.dump(4);
        if (Submission.count(submission_id)) res.set_content("Submission Already Existed!", "text/plain");
        if (Report.count(body["report_id"])) res.set_content("Report_Id Already Existed!", "text/plain");
        Submission.insert(submission_id);
        Report.insert(report_id);
        res.set_content(submission.dump(4), "application/json");
    });
        std::cout << "server on http://127.0.0.1:8000\n";
        server.listen("127.0.0.1", 8000);
    return 0;
}


