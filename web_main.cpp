#include "WebServer.h"
#include <iostream>
#include <signal.h>

WebServer* server = nullptr;

void signalHandler(int signal) {
    if (server) {
        std::cout << "\nShutting down server..." << std::endl;
        server->stop();
    }
    exit(0);
}

int main() {
    signal(SIGINT, signalHandler);
    
    server = new WebServer(8081);  // Port 8081
    
    std::cout << "Starting Options Pricing Engine Web Server..." << std::endl;
    std::cout << "Open your browser and go to: http://localhost:8081" << std::endl;  // Changed to 8081
    std::cout << "Press Ctrl+C to stop the server" << std::endl;
    
    server->start();
    
    delete server;
    return 0;
}
