#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "json.hpp"
#include "simulator.h"

#pragma comment(lib, "Ws2_32.lib")

using namespace std;
using json = nlohmann::json;

string read_file(const string& path) {
    ifstream file(path, ios::binary);
    if (!file.is_open()) return "";
    stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

string get_content_type(const string& path) {
    if (path.find(".html") != string::npos) return "text/html";
    if (path.find(".css") != string::npos) return "text/css";
    if (path.find(".js") != string::npos) return "application/javascript";
    return "text/plain";
}

void send_response(SOCKET clientSocket, const string& status, const string& contentType, const string& body) {
    stringstream response;
    response << "HTTP/1.1 " << status << "\r\n";
    response << "Content-Type: " << contentType << "\r\n";
    response << "Content-Length: " << body.length() << "\r\n";
    response << "Connection: close\r\n\r\n";
    response << body;
    
    string res_str = response.str();
    send(clientSocket, res_str.c_str(), res_str.length(), 0);
}

void handle_client(SOCKET clientSocket) {
    char buffer[8192];
    int bytesReceived = recv(clientSocket, buffer, 8192, 0);
    if (bytesReceived <= 0) {
        closesocket(clientSocket);
        return;
    }
    
    string request(buffer, bytesReceived);
    istringstream requestStream(request);
    string method, path, version;
    requestStream >> method >> path >> version;

    if (method == "GET") {
        if (path == "/") path = "/templates/index.html";
        
        // Remove leading slash
        string filepath = "." + path;
        string content = read_file(filepath);
        
        if (content.empty()) {
            send_response(clientSocket, "404 Not Found", "text/plain", "File Not Found");
        } else {
            send_response(clientSocket, "200 OK", get_content_type(filepath), content);
        }
    } 
    else if (method == "POST" && path == "/api/simulate") {
        // Find body
        size_t body_pos = request.find("\r\n\r\n");
        string body = "";
        if (body_pos != string::npos) {
            body = request.substr(body_pos + 4);
        }
        
        try {
            int num_processes = 20;
            if (!body.empty()) {
                auto j_req = json::parse(body);
                if (j_req.contains("num_processes")) {
                    num_processes = j_req["num_processes"];
                }
            }

            cout << "Simulating " << num_processes << " processes..." << endl;
            vector<Process> processes = gather_real_process_data(num_processes, 10);
            vector<SimulatorResult> sim_results = run_all_simulators(processes);

            json j_response;
            j_response["status"] = "success";
            j_response["data"] = json::array();

            for (const auto& sr : sim_results) {
                json j_sr;
                j_sr["scheduler_name"] = sr.scheduler_name;
                j_sr["avg_waiting_time"] = sr.avg_waiting_time;
                j_sr["avg_turnaround_time"] = sr.avg_turnaround_time;
                
                json j_logs = json::array();
                for (const auto& log : sr.logs) {
                    j_logs.push_back({
                        {"time", log.time},
                        {"pid", log.pid},
                        {"burst_time", log.burst_time},
                        {"waiting_time", log.waiting_time}
                    });
                }
                j_sr["logs"] = j_logs;
                j_response["data"].push_back(j_sr);
            }
            send_response(clientSocket, "200 OK", "application/json", j_response.dump());
        } catch (const exception& e) {
            json j_err;
            j_err["status"] = "error";
            j_err["message"] = e.what();
            send_response(clientSocket, "500 Internal Server Error", "application/json", j_err.dump());
        }
    } else {
        send_response(clientSocket, "405 Method Not Allowed", "text/plain", "Method Not Allowed");
    }
    
    closesocket(clientSocket);
}

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "WSAStartup failed." << endl;
        return 1;
    }

    SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET) {
        cerr << "Socket creation failed." << endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(5000);

    if (bind(listenSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        cerr << "Bind failed." << endl;
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
        cerr << "Listen failed." << endl;
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    cout << "Nexus OS Backend (C++) starting..." << endl;
    cout << "Listening on http://127.0.0.1:5000" << endl;

    while (true) {
        SOCKET clientSocket = accept(listenSocket, NULL, NULL);
        if (clientSocket != INVALID_SOCKET) {
            handle_client(clientSocket);
        }
    }

    closesocket(listenSocket);
    WSACleanup();
    return 0;
}
