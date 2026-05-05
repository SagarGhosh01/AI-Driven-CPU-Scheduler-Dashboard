// ============================================================================
// HTTP SERVER IMPLEMENTATION
// ============================================================================
// Native C++ HTTP server using Windows Sockets API (Winsock2).
// Serves frontend HTML/CSS/JS and provides /api/simulate REST endpoint.
//
// ARCHITECTURE:
// 1. Main server loop: accepts client connections
// 2. Handle client: parses HTTP request and routes to appropriate handler
// 3. Two main handlers:
//    - GET requests: serve static files (HTML/CSS/JS)
//    - POST /api/simulate: run scheduling simulation and return JSON
//
// DEPENDENCIES:
// - winsock2.h: Windows Sockets API for network communication
// - json.hpp: nlohmann/json for JSON parsing and serialization
// - simulator.h: scheduling simulation engine
// ============================================================================

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <vector>
#include <thread>
#include <chrono>
#include <cstring>
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif
#include "json.hpp"
#include "simulator.h"

#ifdef _WIN32
// Link against Winsock2 library
#pragma comment(lib, "Ws2_32.lib")
#else
using SOCKET = int;
constexpr int INVALID_SOCKET = -1;
constexpr int SOCKET_ERROR = -1;
#define closesocket close
#endif

using namespace std;
using json = nlohmann::json;

// ============================================================================
// READ FILE UTILITY
// ============================================================================
// Reads file contents into memory for serving to client
//
// PARAMETERS:
//   path: File path relative to current directory (e.g., "./templates/index.html")
//
// RETURNS: File contents as string, or empty string if file not found
//
// USE CASE: Loading HTML/CSS/JS static files for frontend
// ============================================================================
string read_file(const string& path) {
    // === OPEN FILE IN BINARY MODE ===
    // Binary mode: preserve exact file contents (important for images, etc.)
    ifstream file(path, ios::binary);
    
    // === HANDLE FILE NOT FOUND ===
    if (!file.is_open()) return "";
    
    // === READ ALL CONTENTS INTO STRING ===
    // stringstream: buffer for reading file
    // rdbuf(): read entire file at once
    stringstream ss;
    ss << file.rdbuf();
    
    // === RETURN FILE CONTENTS ===
    return ss.str();
}

// ============================================================================
// GET CONTENT TYPE UTILITY
// ============================================================================
// Determines HTTP Content-Type header based on file extension
//
// PARAMETERS:
//   path: File path (used to check extension)
//
// RETURNS: Content-Type string (e.g., "text/html", "application/json")
//
// USAGE: Sets correct MIME type for HTTP response
// ============================================================================
string get_content_type(const string& path) {
    // Check file extension and return appropriate MIME type
    if (path.find(".html") != string::npos) return "text/html";
    if (path.find(".css") != string::npos) return "text/css";
    if (path.find(".js") != string::npos) return "application/javascript";
    
    // Default fallback for unknown types
    return "text/plain";
}

// ============================================================================
// SEND HTTP RESPONSE
// ============================================================================
// Constructs and sends complete HTTP response to client
//
// PARAMETERS:
//   clientSocket: Socket connected to client
//   status: HTTP status line (e.g., "200 OK", "404 Not Found")
//   contentType: MIME type (e.g., "text/html", "application/json")
//   body: Response body (HTML, JSON, etc.)
//
// STRUCTURE:
// - HTTP status line
// - Content-Type header
// - Content-Length header
// - Connection: close header (single-request connection)
// - Empty line separator
// - Response body
// ============================================================================
void send_response(SOCKET clientSocket, const string& status, const string& contentType, const string& body) {
    // === BUILD RESPONSE HEADERS ===
    stringstream response;
    response << "HTTP/1.1 " << status << "\r\n";                           // Status line
    response << "Content-Type: " << contentType << "\r\n";                 // MIME type
    response << "Content-Length: " << body.length() << "\r\n";             // Body size
    response << "Connection: close\r\n\r\n";                               // Close connection after response
    response << body;                                                       // Response body
    
    // === SEND RESPONSE ===
    string res_str = response.str();
    send(clientSocket, res_str.c_str(), res_str.length(), 0);
}

// ============================================================================
// HANDLE CLIENT REQUEST
// ============================================================================
// Main request handler: parses HTTP request and routes to appropriate handler
//
// REQUEST TYPES:
// 1. GET /  or GET /index.html: serve frontend
// 2. GET /templates/* : serve template files
// 3. GET /static/* : serve CSS/JS assets
// 4. POST /api/simulate: run scheduling simulation
//
// ERROR HANDLING:
// - 404: file not found
// - 405: unsupported HTTP method
// - 500: internal server error (JSON parsing, simulation error)
// ============================================================================
void handle_client(SOCKET clientSocket) {
    // === RECEIVE REQUEST FROM CLIENT ===
    char buffer[8192];
    int bytesReceived = recv(clientSocket, buffer, 8192, 0);
    
    // === HANDLE RECEIVE ERROR ===
    if (bytesReceived <= 0) {
        closesocket(clientSocket);
        return;
    }
    
    // === PARSE HTTP REQUEST LINE ===
    // Format: METHOD PATH VERSION
    // Example: GET /api/simulate HTTP/1.1
    string request(buffer, bytesReceived);
    istringstream requestStream(request);
    string method, path, version;
    requestStream >> method >> path >> version;

    // ====== HANDLE GET REQUESTS ======
    if (method == "GET") {
        // === REDIRECT ROOT TO INDEX.HTML ===
        if (path == "/") path = "/templates/index.html";
        
        // === CONSTRUCT FILE PATH ===
        // Remove leading slash and prepend current directory
        // /templates/index.html → ./templates/index.html
        string filepath = "." + path;
        string content = read_file(filepath);
        
        // === SEND FILE OR 404 ===
        if (content.empty()) {
            send_response(clientSocket, "404 Not Found", "text/plain", "File Not Found");
        } else {
            send_response(clientSocket, "200 OK", get_content_type(filepath), content);
        }
    } 
    // ====== HANDLE POST /API/SIMULATE ======
    else if (method == "POST" && path == "/api/simulate") {
        // === EXTRACT REQUEST BODY ===
        // HTTP format: headers, blank line, body
        // Find the double CRLF that separates headers from body
        size_t body_pos = request.find("\r\n\r\n");
        string body = "";
        if (body_pos != string::npos) {
            body = request.substr(body_pos + 4);
        }
        
        // === TRY TO RUN SIMULATION ===
        try {
            // === PARSE JSON REQUEST ===
            // Default: 20 processes
            int num_processes = 20;
            if (!body.empty()) {
                auto j_req = json::parse(body);
                if (j_req.contains("num_processes")) {
                    num_processes = j_req["num_processes"];
                }
            }

            // === RUN SIMULATION ===
            cout << "Simulating " << num_processes << " processes..." << endl;
            
            // Step 1: Gather real process data from Windows OS
            vector<Process> processes = gather_real_process_data(num_processes, 10);
            
            // Step 2: Run all scheduling algorithms
            vector<SimulatorResult> sim_results = run_all_simulators(processes);

            // === BUILD JSON RESPONSE ===
            json j_response;
            j_response["status"] = "success";
            j_response["data"] = json::array();

            // === ADD RESULTS FOR EACH SCHEDULER ===
            for (const auto& sr : sim_results) {
                json j_sr;
                j_sr["scheduler_name"] = sr.scheduler_name;
                j_sr["avg_waiting_time"] = sr.avg_waiting_time;
                j_sr["avg_turnaround_time"] = sr.avg_turnaround_time;
                
                // === ADD EXECUTION TIMELINE (FOR GANTT CHART) ===
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
            
            // === SEND SUCCESS RESPONSE ===
            send_response(clientSocket, "200 OK", "application/json", j_response.dump());
        } 
        // === HANDLE ERRORS ===
        catch (const exception& e) {
            json j_err;
            j_err["status"] = "error";
            j_err["message"] = e.what();
            send_response(clientSocket, "500 Internal Server Error", "application/json", j_err.dump());
        }
    } 
    // === UNSUPPORTED METHODS ===
    else {
        send_response(clientSocket, "405 Method Not Allowed", "text/plain", "Method Not Allowed");
    }
    
    // === CLOSE CLIENT CONNECTION ===
    closesocket(clientSocket);
}

// ============================================================================
// MAIN SERVER LOOP
// ============================================================================
// Initializes Windows Sockets and starts listening for HTTP requests
// ============================================================================
int main() {
    // === GET PORT FROM ENVIRONMENT VARIABLE ===
    // Default to 5000 if PORT env var not set (for local development)
    int port = 5000;
    const char* port_env = getenv("PORT");
    if (port_env) {
        port = atoi(port_env);
    }

#ifdef _WIN32
    // === INITIALIZE WINDOWS SOCKETS ===
    // MAKEWORD(2, 2): request Winsock version 2.2
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "WSAStartup failed." << endl;
        return 1;
    }
#endif

    // === CREATE LISTENING SOCKET ===
    // AF_INET: IPv4
    // SOCK_STREAM: TCP (connection-oriented)
    SOCKET listenSocket =
#ifdef _WIN32
        socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
#else
        socket(AF_INET, SOCK_STREAM, 0);
#endif
    if (listenSocket == INVALID_SOCKET) {
        cerr << "Socket creation failed." << endl;
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }

    int opt = 1;
    setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&opt), sizeof(opt));

    // === BIND SOCKET TO PORT ===
    // Bind to all interfaces (INADDR_ANY) on dynamic port from environment
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;        // Listen on all interfaces
    serverAddr.sin_port = htons(port);               // Port from environment or default 5000

    if (bind(listenSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        cerr << "Bind failed." << endl;
        closesocket(listenSocket);
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }

    // === LISTEN FOR INCOMING CONNECTIONS ===
    // SOMAXCONN: maximum queue for pending connections
    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
        cerr << "Listen failed." << endl;
        closesocket(listenSocket);
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }

    // === PRINT SERVER STARTUP MESSAGE ===
    cout << "Nexus OS Backend (C++) starting..." << endl;
    cout << "Listening on http://0.0.0.0:" << port << endl;

    // === MAIN ACCEPT LOOP ===
    // Accept client connections in a loop (infinite)
    while (true) {
        // === ACCEPT INCOMING CONNECTION ===
        SOCKET clientSocket = accept(listenSocket, NULL, NULL);
        
        // === HANDLE CLIENT ===
        if (clientSocket != INVALID_SOCKET) {
            handle_client(clientSocket);  // Process the request
        }
    }

    // === CLEANUP (UNREACHABLE IN INFINITE LOOP) ===
    closesocket(listenSocket);
#ifdef _WIN32
    WSACleanup();
#endif
    return 0;
}
