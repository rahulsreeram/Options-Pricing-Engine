#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include "OptionsPricingEngine.h"
#include <string>
#include <map>

class WebServer {
private:
    OptionsPricingEngine engine_;
    int port_;
    bool running_;
    
    std::string generateHTML();
    std::string handlePricingRequest(const std::map<std::string, std::string>& params);
    std::string generatePayoffData(const std::map<std::string, std::string>& params);
    std::string generateStrategyData(const std::map<std::string, std::string>& params);
    std::map<std::string, std::string> parseQueryString(const std::string& query);
    void sendHTTPResponse(int client_socket, const std::string& content, const std::string& contentType = "text/html");
    
public:
    WebServer(int port = 8081);
    void start();
    void stop();
    void handleClient(int client_socket);
};

#endif
