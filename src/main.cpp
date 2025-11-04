#include <httplib.h>
#include <iostream>
#include"config_manager.hpp"
#include<nlohmann/json.hpp>
#include<unordered_set>
#include<fstream>
#include<ctime>
using json = nlohmann::json;

int main() {
    httplib::Server server;  // Create a server object
    ConfigManager CM;
    CM.loadConfig("configs/default.json");
    int score = 0;
    std::unordered_set<std::string> evaluated_ids;
    // Define an endpoint: when user goes to /hello, send "Hello World"
    server.Get("/api/config", [&](const httplib::Request& req,httplib::Response& res) {
        res.set_content(CM.getConfigJSON().dump(4), "application/json");
    });
    server.Get("/api/evaluation", [&](const httplib::Request& req, httplib::Response& res) {
        json data = CM.getConfigJSON();
        json eval = data["evaluation"];  //  evaluation section
        res.set_content(eval.dump(4), "application/json");
    });
    server.Post(R"(/api/evaluation/(\w+))", [&](const httplib::Request& req, httplib::Response& res){
            std::string id = req.matches[1];
            if(evaluated_ids.count(id)) {
                res.set_content("ID already evaluated", "text/plain");
                return;
            }
            evaluated_ids.insert(id);
            json rule = CM.getRuleById(id, score);

            res.set_content(rule.dump(4), "application/json");
    });

    server.Post("/api/scores", [&](const httplib::Request& req, httplib::Response& res){
        std::string result = CM.printEvaluations(score);
        json Response_Score;
        Response_Score["status"] = result;
        Response_Score["Total_Score"] = score;
        res.set_content(Response_Score.dump(4), "application/json");
    });

    server.Post("/api/echo", [&](const httplib::Request& req, httplib::Response& res) {
        res.set_content(req.body, "application/json");
    });
    server.Post("/api/cls", [&](const httplib::Request& req, httplib::Response& res){
        score = 0;
        evaluated_ids.clear();
        res.set_content("Score Reset To 0", "text/plain");
    });
    std::cout << "🚀 Server running at http://127.0.0.1:8000/api/config\n";

    /*submission id*/

    server.Post(R"(/(\d+)/reports)", [](const httplib::Request& req, httplib::Response& res) {
        int submission_id = std::stoi(req.matches[1]);
        std::cout << ">>> HIT ROUTE " << submission_id << " <<<" << std::endl;
        
        std::unordered_set<int> Check_id_submit;
        if (Check_id_submit.count(submission_id)) {
            std::cerr << "Already Submitted A Report With That ID" << std::endl;
            res.set_content("ID Already Submitted", "text/plain");
        }
        Check_id_submit.insert(submission_id);
        json body = json::parse(req.body);
        if (!body.contains("submission_id") || !body["submission_id"].is_number() || submission_id != body["submission_id"]) {
            res.status = 400;
            res.set_content("submission_id must be number Or Some Errors Occured With Current ID, Please Recheck", "text/plain");
            return; 
        }
        std::string PATH = "data/" + std::to_string(submission_id) + ".json";
        // lay thoi gian thuc o hien tai //
        std::time_t now = std::time(nullptr);
        std::tm *TM = std::localtime(&now);
        int nam  = TM->tm_year + 1900;
        int thang = TM->tm_mon + 1;
        int ngay   = TM->tm_mday;
        std::string Date = std::to_string(nam) + "/" +
                            std::to_string(thang) + "/" +
                            std::to_string(ngay);
        body["created_on"] = Date;
        // tao file //
        std::ofstream new_file(PATH);
        new_file << body.dump(4);
        new_file.close();
        std::cout << "New Submission (ID: " << submission_id << ") " << "Created!" << std::endl;
        res.set_content("New Submission Created: " + PATH, "text/plain");
    });
    server.Get(R"(/(\d+)/reports)", [](const httplib::Request& req, httplib::Response& res){
        /*debug*/
        int id = std::stoi(req.matches[1]);
        std::cout << "Get Submission Report (id: " << id << ") " << std::endl;
        std::string PATH = "data/" + std::to_string(id) + ".json";
        std::ifstream f(PATH);
        if(!f.is_open()){
            res.status = 404;
            res.set_content("Report not found", "text/plain");
            return;
        }
        json body;
        f >> body;
        res.set_content(body.dump(4),"application/json");
    }); 
    // Start listening on localhost port 8000
    server.listen("127.0.0.1", 8000);

}