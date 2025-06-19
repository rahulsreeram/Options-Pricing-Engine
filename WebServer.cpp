#include "WebServer.h"
#include <iostream>
#include <sstream>
#include <thread>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <cmath>
#include <iomanip>

WebServer::WebServer(int port) : port_(port), running_(false) {
    engine_.setBinomialSteps(1000);
    engine_.setMonteCarloSimulations(100000);
}

void WebServer::start() {
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        std::cerr << "Error creating socket" << std::endl;
        return;
    }
    
    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port_);
    
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Error binding socket" << std::endl;
        close(server_socket);
        return;
    }
    
    if (listen(server_socket, 5) < 0) {
        std::cerr << "Error listening on socket" << std::endl;
        close(server_socket);
        return;
    }
    
    running_ = true;
    std::cout << "Web server started on http://localhost:" << port_ << std::endl;
    
    while (running_) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
        
        if (client_socket >= 0) {
            std::thread client_thread(&WebServer::handleClient, this, client_socket);
            client_thread.detach();
        }
    }
    
    close(server_socket);
}

void WebServer::handleClient(int client_socket) {
    char buffer[4096];
    ssize_t bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    
    if (bytes_received > 0) {
        buffer[bytes_received] = '\0';
        std::string request(buffer);
        
        if (request.find("GET / ") == 0 || request.find("GET /index.html") == 0) {
            sendHTTPResponse(client_socket, generateHTML());
        } else if (request.find("GET /calculate?") != std::string::npos) {
            size_t query_start = request.find("?") + 1;
            size_t query_end = request.find(" ", query_start);
            std::string query = request.substr(query_start, query_end - query_start);
            
            auto params = parseQueryString(query);
            std::string result = handlePricingRequest(params);
            sendHTTPResponse(client_socket, result, "application/json");
        } else if (request.find("GET /payoff-data?") != std::string::npos) {
            size_t query_start = request.find("?") + 1;
            size_t query_end = request.find(" ", query_start);
            std::string query = request.substr(query_start, query_end - query_start);
            
            auto params = parseQueryString(query);
            std::string result = generatePayoffData(params);
            sendHTTPResponse(client_socket, result, "application/json");
        } else if (request.find("GET /strategy-data?") != std::string::npos) {
            size_t query_start = request.find("?") + 1;
            size_t query_end = request.find(" ", query_start);
            std::string query = request.substr(query_start, query_end - query_start);
            
            auto params = parseQueryString(query);
            std::string result = generateStrategyData(params);
            sendHTTPResponse(client_socket, result, "application/json");
        } else {
            sendHTTPResponse(client_socket, "<h1>404 Not Found</h1>");
        }
    }
    
    close(client_socket);
}

std::string WebServer::generateHTML() {
    std::string html = "<!DOCTYPE html>\n";
    html += "<html lang=\"en\">\n";
    html += "<head>\n";
    html += "<meta charset=\"UTF-8\">\n";
    html += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n";
    html += "<title>Advanced Options Trading Engine</title>\n";
    html += "<script src=\"https://cdn.jsdelivr.net/npm/chart.js\"></script>\n";
    html += "<style>\n";
    html += "* { margin: 0; padding: 0; box-sizing: border-box; }\n";
    html += "body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); min-height: 100vh; padding: 20px; }\n";
    html += ".container { max-width: 1400px; margin: 0 auto; background: rgba(255, 255, 255, 0.95); border-radius: 20px; box-shadow: 0 20px 40px rgba(0, 0, 0, 0.1); overflow: hidden; }\n";
    html += ".header { background: linear-gradient(135deg, #1e3c72 0%, #2a5298 100%); color: white; padding: 30px; text-align: center; }\n";
    html += ".header h1 { font-size: 2.5em; margin-bottom: 10px; font-weight: 300; }\n";
    html += ".header p { opacity: 0.9; font-size: 1.1em; }\n";
    html += ".main-content { display: grid; grid-template-columns: 350px 1fr; gap: 30px; padding: 40px; }\n";
    html += ".form-section { background: rgba(255, 255, 255, 0.9); padding: 30px; border-radius: 15px; box-shadow: 0 10px 25px rgba(0, 0, 0, 0.1); }\n";
    html += ".chart-section { background: rgba(255, 255, 255, 0.9); padding: 30px; border-radius: 15px; box-shadow: 0 10px 25px rgba(0, 0, 0, 0.1); }\n";
    html += ".input-group { margin-bottom: 15px; }\n";
    html += ".input-group label { display: block; margin-bottom: 5px; font-weight: 600; color: #333; font-size: 13px; }\n";
    html += ".input-group input, .input-group select { width: 100%; padding: 10px; border: 2px solid #e1e8ed; border-radius: 6px; font-size: 13px; background: #f8f9fa; transition: all 0.3s ease; }\n";
    html += ".input-group input:focus, .input-group select:focus { outline: none; border-color: #667eea; background: white; box-shadow: 0 0 0 3px rgba(102, 126, 234, 0.1); }\n";
    html += ".btn-calculate { background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); color: white; border: none; padding: 12px 25px; font-size: 14px; font-weight: 600; border-radius: 20px; cursor: pointer; width: 100%; margin-top: 15px; transition: all 0.3s ease; }\n";
    html += ".btn-calculate:hover { transform: translateY(-2px); box-shadow: 0 12px 20px rgba(102, 126, 234, 0.4); }\n";
    html += ".chart-container { position: relative; height: 500px; margin-bottom: 30px; }\n";
    html += ".chart-controls { display: flex; gap: 8px; margin-bottom: 20px; flex-wrap: wrap; }\n";
    html += ".chart-btn { background: #667eea; color: white; border: none; padding: 8px 16px; border-radius: 15px; cursor: pointer; font-size: 12px; transition: all 0.3s ease; }\n";
    html += ".chart-btn:hover { background: #5a6fd8; transform: translateY(-1px); }\n";
    html += ".chart-btn.active { background: #4c63d2; box-shadow: 0 4px 8px rgba(102, 126, 234, 0.3); }\n";
    html += ".strategy-section { background: rgba(255, 255, 255, 0.9); padding: 20px; border-radius: 15px; margin-top: 20px; }\n";
    html += ".strategy-controls { display: flex; gap: 10px; margin-bottom: 15px; flex-wrap: wrap; }\n";
    html += ".strategy-btn { background: #28a745; color: white; border: none; padding: 10px 16px; border-radius: 20px; cursor: pointer; font-size: 13px; font-weight: 600; transition: all 0.3s ease; }\n";
    html += ".strategy-btn:hover { background: #218838; transform: translateY(-1px); }\n";
    html += ".strategy-btn.active { background: #1e7e34; box-shadow: 0 4px 8px rgba(40, 167, 69, 0.3); }\n";
    html += ".analysis-section { background: linear-gradient(135deg, #f093fb 0%, #f5576c 100%); color: white; padding: 25px; border-radius: 15px; margin-top: 20px; }\n";
    html += ".analysis-grid { display: grid; grid-template-columns: 1fr 1fr 1fr; gap: 15px; margin-top: 15px; }\n";
    html += ".analysis-card { background: rgba(255, 255, 255, 0.2); padding: 15px; border-radius: 10px; backdrop-filter: blur(10px); }\n";
    html += ".analysis-item { display: flex; justify-content: space-between; margin-bottom: 8px; padding: 3px 0; border-bottom: 1px solid rgba(255, 255, 255, 0.2); font-size: 13px; }\n";
    html += ".analysis-item:last-child { border-bottom: none; }\n";
    html += ".analysis-value { font-family: 'Courier New', monospace; font-weight: bold; }\n";
    html += ".results-display { background: rgba(102, 126, 234, 0.1); padding: 15px; border-radius: 10px; margin-top: 15px; color: #333; }\n";
    html += ".price-item { display: flex; justify-content: space-between; margin: 5px 0; font-size: 13px; }\n";
    html += ".strategy-info { background: rgba(40, 167, 69, 0.1); padding: 15px; border-radius: 10px; margin-top: 15px; color: #333; }\n";
    html += ".strategy-details { font-size: 13px; line-height: 1.4; }\n";
    html += ".position-toggle { display: flex; gap: 10px; margin-bottom: 20px; }\n";
    html += ".toggle-btn { background: #6c757d; color: white; border: none; padding: 10px 20px; border-radius: 20px; cursor: pointer; font-size: 14px; font-weight: 600; transition: all 0.3s ease; }\n";
    html += ".toggle-btn.active { background: #007bff; box-shadow: 0 4px 8px rgba(0, 123, 255, 0.3); }\n";
    html += ".toggle-btn:hover { transform: translateY(-1px); }\n";
    html += ".strategy-position-toggle { display: flex; gap: 8px; margin-top: 10px; }\n";
    html += ".strategy-toggle-btn { background: #6c757d; color: white; border: none; padding: 6px 12px; border-radius: 15px; cursor: pointer; font-size: 11px; font-weight: 600; transition: all 0.3s ease; }\n";
    html += ".strategy-toggle-btn.active { background: #dc3545; }\n";
    html += ".strategy-toggle-btn.long.active { background: #28a745; }\n";
    html += ".hidden { display: none; }\n";
    html += "@media (max-width: 1200px) { .main-content { grid-template-columns: 1fr; } .analysis-grid { grid-template-columns: 1fr 1fr; } }\n";
    html += "@media (max-width: 768px) { .analysis-grid { grid-template-columns: 1fr; } }\n";
    html += "</style>\n";
    html += "</head>\n";
    html += "<body>\n";
    
    html += "<div class=\"container\">\n";
    html += "<div class=\"header\">\n";
    html += "<h1>Advanced Options Trading Engine</h1>\n";
    html += "<p>Professional options analysis with buying, selling, and strategy combinations</p>\n";
    html += "</div>\n";
    
    html += "<div class=\"main-content\">\n";
    html += "<div class=\"form-section\">\n";
    html += "<h2 style=\"margin-bottom: 20px; color: #333; text-align: center; font-size: 1.3em;\">Option Parameters</h2>\n";
    
    // Buy/Sell Toggle for single options
    html += "<div class=\"position-toggle\">\n";
    html += "<button class=\"toggle-btn active\" onclick=\"setPositionType('long')\">Long (Buying)</button>\n";
    html += "<button class=\"toggle-btn\" onclick=\"setPositionType('short')\">Short (Selling)</button>\n";
    html += "</div>\n";
    
    html += "<form id=\"pricingForm\">\n";
    html += "<div class=\"input-group\">\n";
    html += "<label for=\"spot\">Current Stock Price ($)</label>\n";
    html += "<input type=\"number\" id=\"spot\" name=\"spot\" value=\"100\" step=\"0.01\" required>\n";
    html += "</div>\n";
    html += "<div class=\"input-group\">\n";
    html += "<label for=\"strike\">Strike Price ($)</label>\n";
    html += "<input type=\"number\" id=\"strike\" name=\"strike\" value=\"105\" step=\"0.01\" required>\n";
    html += "</div>\n";
    html += "<!-- Second strike - hidden by default -->\n";
    html += "<div class=\"input-group hidden\" id=\"strike2Group\">\n";
    html += "<label for=\"strike2\">Strike Price 2 ($)</label>\n";
    html += "<input type=\"number\" id=\"strike2\" name=\"strike2\" value=\"110\" step=\"0.01\">\n";
    html += "</div>\n";
    html += "<div class=\"input-group\">\n";
    html += "<label for=\"rate\">Risk-free Rate (%)</label>\n";
    html += "<input type=\"number\" id=\"rate\" name=\"rate\" value=\"5\" step=\"0.1\" required>\n";
    html += "</div>\n";
    html += "<div class=\"input-group\">\n";
    html += "<label for=\"volatility\">Implied Volatility (%)</label>\n";
    html += "<input type=\"number\" id=\"volatility\" name=\"volatility\" value=\"20\" step=\"0.1\" required>\n";
    html += "</div>\n";
    html += "<div class=\"input-group\">\n";
    html += "<label for=\"timeToMaturity\">Days to Expiration</label>\n";
    html += "<input type=\"number\" id=\"timeToMaturity\" name=\"timeToMaturity\" value=\"90\" step=\"1\" required>\n";
    html += "</div>\n";
    html += "<div class=\"input-group\">\n";
    html += "<label for=\"optionType\">Option Type</label>\n";
    html += "<select id=\"optionType\" name=\"optionType\" required>\n";
    html += "<option value=\"0\">Call Option</option>\n";
    html += "<option value=\"1\">Put Option</option>\n";
    html += "</select>\n";
    html += "</div>\n";
    html += "<div class=\"input-group\">\n";
    html += "<label for=\"exerciseType\">Exercise Style</label>\n";
    html += "<select id=\"exerciseType\" name=\"exerciseType\" required>\n";
    html += "<option value=\"0\">European</option>\n";
    html += "<option value=\"1\">American</option>\n";
    html += "</select>\n";
    html += "</div>\n";
    
    html += "<button type=\"button\" class=\"btn-calculate\" onclick=\"calculateAndChart()\">Calculate & Visualize</button>\n";
    html += "</form>\n";
    
    html += "<div class=\"strategy-section\">\n";
    html += "<h3 style=\"margin-bottom: 15px; color: #333;\">Quick Strategies</h3>\n";
    html += "<div class=\"strategy-controls\">\n";
    html += "<button class=\"strategy-btn\" onclick=\"loadStrategy('straddle')\">Straddle</button>\n";
    html += "<button class=\"strategy-btn\" onclick=\"loadStrategy('strangle')\">Strangle</button>\n";
    html += "<button class=\"strategy-btn\" onclick=\"loadStrategy('coveredcall')\">Covered Call</button>\n";
    html += "<button class=\"strategy-btn\" onclick=\"loadStrategy('protectiveput')\">Protective Put</button>\n";
    html += "<button class=\"strategy-btn\" onclick=\"loadStrategy('bullspread')\">Bull Call Spread</button>\n";
    html += "<button class=\"strategy-btn\" onclick=\"loadStrategy('bearspread')\">Bear Put Spread</button>\n";
    html += "<button class=\"strategy-btn\" onclick=\"loadStrategy('ironcondor')\">Iron Condor</button>\n";
    html += "</div>\n";
    html += "<div id=\"strategyInfo\"></div>\n";
    html += "</div>\n";
    
    html += "<div id=\"pricingResults\"></div>\n";
    html += "</div>\n";
    
    html += "<div class=\"chart-section\">\n";
    html += "<h2 style=\"margin-bottom: 20px; color: #333;\">Interactive P&L Analysis</h2>\n";
    html += "<div class=\"chart-controls\">\n";
    html += "<button class=\"chart-btn active\" onclick=\"showChart('payoff')\">Payoff</button>\n";
    html += "<button class=\"chart-btn\" onclick=\"showChart('pnl')\">P&L</button>\n";
    html += "<button class=\"chart-btn\" onclick=\"showChart('breakeven')\">Breakeven</button>\n";
    html += "<button class=\"chart-btn\" onclick=\"showChart('comparison')\">Buy vs Sell</button>\n";
    html += "<button class=\"chart-btn\" onclick=\"showChart('greeks')\">Greeks</button>\n";
    html += "</div>\n";
    html += "<div class=\"chart-container\">\n";
    html += "<canvas id=\"payoffChart\"></canvas>\n";
    html += "</div>\n";
    html += "<div id=\"analysisResults\"></div>\n";
    html += "</div>\n";
    html += "</div>\n";
    html += "</div>\n";
    
    // Complete JavaScript with all fixes
    html += "<script>\n";
    html += "let payoffChart = null;\n";
    html += "let currentChartType = 'payoff';\n";
    html += "let currentOptionData = null;\n";
    html += "let pricingData = null;\n";
    html += "let currentStrategy = null;\n";
    html += "let currentPosition = 'long';\n";
    html += "let strategyPosition = 'long';\n";
    html += "\n";
    html += "const strategies = {\n";
    html += "  straddle: { name: 'Straddle', needsSecondStrike: false, canGoShort: true },\n";
    html += "  strangle: { name: 'Strangle', needsSecondStrike: true, canGoShort: true },\n";
    html += "  coveredcall: { name: 'Covered Call', needsSecondStrike: false, canGoShort: false },\n";
    html += "  protectiveput: { name: 'Protective Put', needsSecondStrike: false, canGoShort: false },\n";
    html += "  bullspread: { name: 'Bull Call Spread', needsSecondStrike: true, canGoShort: true },\n";
    html += "  bearspread: { name: 'Bear Put Spread', needsSecondStrike: true, canGoShort: true },\n";
    html += "  ironcondor: { name: 'Iron Condor', needsSecondStrike: true, canGoShort: false }\n";
    html += "};\n";
    html += "\n";
    html += "function initChart() {\n";
    html += "  const ctx = document.getElementById('payoffChart').getContext('2d');\n";
    html += "  payoffChart = new Chart(ctx, {\n";
    html += "    type: 'line',\n";
    html += "    data: { labels: [], datasets: [] },\n";
    html += "    options: {\n";
    html += "      responsive: true,\n";
    html += "      maintainAspectRatio: false,\n";
    html += "      interaction: { \n";
    html += "        mode: 'index', \n";
    html += "        intersect: false \n";
    html += "      },\n";
    html += "      scales: {\n";
    html += "        x: { \n";
    html += "          title: { display: true, text: 'Underlying Price ($)' },\n";
    html += "          grid: { color: 'rgba(0, 0, 0, 0.1)' }\n";
    html += "        },\n";
    html += "        y: { \n";
    html += "          title: { display: true, text: 'Profit/Loss ($)' },\n";
    html += "          grid: { color: 'rgba(0, 0, 0, 0.1)' }\n";
    html += "        }\n";
    html += "      },\n";
    html += "      plugins: {\n";
    html += "        tooltip: {\n";
    html += "          mode: 'index',\n";
    html += "          intersect: false,\n";
    html += "          backgroundColor: 'rgba(0, 0, 0, 0.8)',\n";
    html += "          titleColor: 'white',\n";
    html += "          bodyColor: 'white',\n";
    html += "          borderColor: 'rgba(255, 255, 255, 0.3)',\n";
    html += "          borderWidth: 1,\n";
    html += "          callbacks: {\n";
    html += "            title: function(context) {\n";
    html += "              return 'Price: $' + parseFloat(context[0].label).toFixed(2);\n";
    html += "            },\n";
    html += "            label: function(context) {\n";
    html += "              const datasetLabel = context.dataset.label || '';\n";
    html += "              const value = context.parsed.y;\n";
    html += "              \n";
    html += "              if (datasetLabel.includes('Breakeven')) {\n";
    html += "                return 'Breakeven Point: $' + context.parsed.x.toFixed(2);\n";
    html += "              }\n";
    html += "              \n";
    html += "              return datasetLabel + ': $' + value.toFixed(2);\n";
    html += "            },\n";
    html += "            afterBody: function(context) {\n";
    html += "              const price = parseFloat(context[0].label);\n";
    html += "              const pnlValue = context[0].parsed.y;\n";
    html += "              \n";
    html += "              let info = [];\n";
    html += "              \n";
    html += "              if (pricingData && (pricingData.breakeven_long || pricingData.breakeven_short || pricingData.breakevens)) {\n";
    html += "                if (pricingData.breakevens) {\n";
    html += "                  const closestBreakeven = pricingData.breakevens.reduce((prev, curr) => \n";
    html += "                    Math.abs(curr - price) < Math.abs(prev - price) ? curr : prev);\n";
    html += "                  const distance = Math.abs(price - closestBreakeven);\n";
    html += "                  \n";
    html += "                  if (distance < 0.5) {\n";
    html += "                    info.push('Near Breakeven Point');\n";
    html += "                  } else {\n";
    html += "                    info.push('Distance to Breakeven: $' + distance.toFixed(2));\n";
    html += "                  }\n";
    html += "                } else {\n";
    html += "                  const breakeven = currentPosition === 'long' ? pricingData.breakeven_long : pricingData.breakeven_short;\n";
    html += "                  const distance = Math.abs(price - breakeven);\n";
    html += "                  \n";
    html += "                  if (distance < 0.5) {\n";
    html += "                    info.push('Near Breakeven Point');\n";
    html += "                  } else {\n";
    html += "                    info.push('Distance to Breakeven: $' + distance.toFixed(2));\n";
    html += "                  }\n";
    html += "                }\n";
    html += "              }\n";
    html += "              \n";
    html += "              if (pnlValue > 0) {\n";
    html += "                info.push('Profitable Position');\n";
    html += "              } else if (pnlValue < 0) {\n";
    html += "                info.push('Loss Position');\n";
    html += "              } else {\n";
    html += "                info.push('Breakeven Position');\n";
    html += "              }\n";
    html += "              \n";
    html += "              return info;\n";
    html += "            }\n";
    html += "          }\n";
    html += "        },\n";
    html += "        legend: {\n";
    html += "          display: true,\n";
    html += "          position: 'top'\n";
    html += "        }\n";
    html += "      }\n";
    html += "    }\n";
    html += "  });\n";
    html += "}\n";
    html += "\n";
    html += "function setPositionType(type) {\n";
    html += "  currentPosition = type;\n";
    html += "  document.querySelectorAll('.toggle-btn').forEach(btn => btn.classList.remove('active'));\n";
    html += "  event.target.classList.add('active');\n";
    html += "  \n";
    html += "  currentStrategy = null;\n";
    html += "  strategyPosition = 'long';\n";
    html += "  document.querySelectorAll('.strategy-btn').forEach(btn => btn.classList.remove('active'));\n";
    html += "  document.getElementById('strategyInfo').innerHTML = '';\n";
    html += "  document.getElementById('strike2Group').classList.add('hidden');\n";
    html += "  \n";
    html += "  if (pricingData) {\n";
    html += "    updateChart(pricingData);\n";
    html += "    showResults(pricingData);\n";
    html += "  }\n";
    html += "}\n";
    html += "\n";
    html += "function setStrategyPosition(type) {\n";
    html += "  strategyPosition = type;\n";
    html += "  document.querySelectorAll('.strategy-toggle-btn').forEach(btn => btn.classList.remove('active'));\n";
    html += "  event.target.classList.add('active');\n";
    html += "  \n";
    html += "  if (currentStrategy && pricingData) {\n";
    html += "    calculateAndChart();\n";
    html += "  }\n";
    html += "}\n";
    html += "\n";
    html += "function loadStrategy(strategyType) {\n";
    html += "  currentStrategy = strategyType;\n";
    html += "  strategyPosition = 'long';\n";
    html += "  \n";
    html += "  document.querySelectorAll('.strategy-btn').forEach(btn => btn.classList.remove('active'));\n";
    html += "  event.target.classList.add('active');\n";
    html += "  \n";
    html += "  const strategy = strategies[strategyType];\n";
    html += "  const strike2Group = document.getElementById('strike2Group');\n";
    html += "  \n";
    html += "  if (strategy.needsSecondStrike) {\n";
    html += "    strike2Group.classList.remove('hidden');\n";
    html += "    document.getElementById('strike2').required = true;\n";
    html += "    \n";
    html += "    const strike1 = parseFloat(document.getElementById('strike').value);\n";
    html += "    if (strategyType === 'strangle') {\n";
    html += "      document.getElementById('strike2').value = strike1 + 5;\n";
    html += "    } else if (strategyType === 'bullspread' || strategyType === 'bearspread') {\n";
    html += "      document.getElementById('strike2').value = strike1 + 10;\n";
    html += "    } else if (strategyType === 'ironcondor') {\n";
    html += "      document.getElementById('strike2').value = strike1 + 15;\n";
    html += "    }\n";
    html += "  } else {\n";
    html += "    strike2Group.classList.add('hidden');\n";
    html += "    document.getElementById('strike2').required = false;\n";
    html += "  }\n";
    html += "  \n";
    html += "  let strategyToggle = '';\n";
    html += "  if (strategy.canGoShort) {\n";
    html += "    strategyToggle = `\n";
    html += "      <div class=\"strategy-position-toggle\">\n";
    html += "        <button class=\"strategy-toggle-btn long active\" onclick=\"setStrategyPosition('long')\">Long ${strategy.name}</button>\n";
    html += "        <button class=\"strategy-toggle-btn short\" onclick=\"setStrategyPosition('short')\">Short ${strategy.name}</button>\n";
    html += "      </div>\n";
    html += "    `;\n";
    html += "  }\n";
    html += "  \n";
    html += "  const infoHtml = `\n";
    html += "    <div class=\"strategy-info\">\n";
    html += "      <h4>${strategy.name}</h4>\n";
    html += "      <div class=\"strategy-details\">\n";
    html += "        <p><strong>Strategy selected:</strong> ${strategy.name}</p>\n";
    html += "        ${strategy.needsSecondStrike ? '<p><strong>Note:</strong> This strategy uses two strikes. Adjust Strike 2 as needed.</p>' : ''}\n";
    html += "        ${strategyToggle}\n";
    html += "        <p style=\"margin-top: 10px;\">Click \"Calculate & Visualize\" to see the strategy payoff diagram.</p>\n";
    html += "      </div>\n";
    html += "    </div>\n";
    html += "  `;\n";
    html += "  \n";
    html += "  document.getElementById('strategyInfo').innerHTML = infoHtml;\n";
    html += "}\n";
    html += "\n";
    html += "function calculateAndChart() {\n";
    html += "  const formData = new FormData(document.getElementById('pricingForm'));\n";
    html += "  const params = new URLSearchParams();\n";
    html += "  \n";
    html += "  const rate = parseFloat(formData.get('rate')) / 100;\n";
    html += "  const volatility = parseFloat(formData.get('volatility')) / 100;\n";
    html += "  const timeToMaturity = parseFloat(formData.get('timeToMaturity')) / 365;\n";
    html += "  \n";
    html += "  currentOptionData = {\n";
    html += "    spot: parseFloat(formData.get('spot')),\n";
    html += "    strike: parseFloat(formData.get('strike')),\n";
    html += "    rate: rate,\n";
    html += "    volatility: volatility,\n";
    html += "    timeToMaturity: timeToMaturity,\n";
    html += "    optionType: parseInt(formData.get('optionType')),\n";
    html += "    exerciseType: parseInt(formData.get('exerciseType'))\n";
    html += "  };\n";
    html += "  \n";
    html += "  params.append('spot', formData.get('spot'));\n";
    html += "  params.append('strike', formData.get('strike'));\n";
    html += "  params.append('rate', rate);\n";
    html += "  params.append('volatility', volatility);\n";
    html += "  params.append('timeToMaturity', timeToMaturity);\n";
    html += "  params.append('optionType', formData.get('optionType'));\n";
    html += "  params.append('exerciseType', formData.get('exerciseType'));\n";
    html += "  params.append('position', currentPosition);\n";
    html += "  \n";
    html += "  if (currentStrategy) {\n";
    html += "    params.append('strategy', currentStrategy);\n";
    html += "    params.append('strategyPosition', strategyPosition);\n";
    html += "    if (strategies[currentStrategy].needsSecondStrike) {\n";
    html += "      params.append('strike2', formData.get('strike2'));\n";
    html += "    }\n";
    html += "    \n";
    html += "    fetch('/strategy-data?' + params.toString())\n";
    html += "      .then(response => response.json())\n";
    html += "      .then(data => {\n";
    html += "        pricingData = data;\n";
    html += "        updateStrategyChart(data);\n";
    html += "        showStrategyResults(data);\n";
    html += "      })\n";
    html += "      .catch(error => console.error('Error:', error));\n";
    html += "  } else {\n";
    html += "    fetch('/payoff-data?' + params.toString())\n";
    html += "      .then(response => response.json())\n";
    html += "      .then(data => {\n";
    html += "        pricingData = data;\n";
    html += "        updateChart(data);\n";
    html += "        showResults(data);\n";
    html += "      })\n";
    html += "      .catch(error => console.error('Error:', error));\n";
    html += "  }\n";
    html += "}\n";
    html += "\n";
    html += "function updateChart(data) {\n";
    html += "  if (!payoffChart || !data) return;\n";
    html += "  \n";
    html += "  payoffChart.data.labels = data.prices;\n";
    html += "  payoffChart.data.datasets = [];\n";
    html += "  \n";
    html += "  if (currentChartType === 'comparison') {\n";
    html += "    payoffChart.data.datasets = [\n";
    html += "      {\n";
    html += "        label: 'Long Position P&L',\n";
    html += "        data: data.pnl_long,\n";
    html += "        borderColor: '#28a745',\n";
    html += "        backgroundColor: 'rgba(40, 167, 69, 0.1)',\n";
    html += "        borderWidth: 3,\n";
    html += "        fill: false,\n";
    html += "        tension: 0.1\n";
    html += "      },\n";
    html += "      {\n";
    html += "        label: 'Short Position P&L',\n";
    html += "        data: data.pnl_short,\n";
    html += "        borderColor: '#dc3545',\n";
    html += "        backgroundColor: 'rgba(220, 53, 69, 0.1)',\n";
    html += "        borderWidth: 3,\n";
    html += "        fill: false,\n";
    html += "        tension: 0.1\n";
    html += "      }\n";
    html += "    ];\n";
    html += "  } else {\n";
    html += "    const pnlData = currentPosition === 'long' ? data.pnl_long : data.pnl_short;\n";
    html += "    const color = currentPosition === 'long' ? '#28a745' : '#dc3545';\n";
    html += "    \n";
    html += "    payoffChart.data.datasets.push({\n";
    html += "      label: currentPosition === 'long' ? 'Long Position P&L' : 'Short Position P&L',\n";
    html += "      data: pnlData,\n";
    html += "      borderColor: color,\n";
    html += "      backgroundColor: color === '#28a745' ? 'rgba(40, 167, 69, 0.1)' : 'rgba(220, 53, 69, 0.1)',\n";
    html += "      borderWidth: 3,\n";
    html += "      fill: true,\n";
    html += "      tension: 0.1\n";
    html += "    });\n";
    html += "  }\n";
    html += "  \n";
    html += "  const breakeven = currentPosition === 'long' ? data.breakeven_long : data.breakeven_short;\n";
    html += "  if (breakeven) {\n";
    html += "    payoffChart.data.datasets.push({\n";
    html += "      label: 'Breakeven Point',\n";
    html += "      data: [{ x: breakeven, y: 0 }],\n";
    html += "      borderColor: '#ff6b6b',\n";
    html += "      backgroundColor: '#ff6b6b',\n";
    html += "      pointRadius: 8,\n";
    html += "      pointHoverRadius: 10,\n";
    html += "      showLine: false\n";
    html += "    });\n";
    html += "  }\n";
    html += "  \n";
    html += "  payoffChart.update();\n";
    html += "}\n";
    html += "\n";
    html += "function updateStrategyChart(data) {\n";
    html += "  if (!payoffChart || !data) return;\n";
    html += "  \n";
    html += "  payoffChart.data.labels = data.prices;\n";
    html += "  payoffChart.data.datasets = [];\n";
    html += "  \n";
    html += "  const strategyLabel = (strategyPosition === 'long' ? 'Long ' : 'Short ') + strategies[currentStrategy].name;\n";
    html += "  const color = strategyPosition === 'long' ? '#28a745' : '#dc3545';\n";
    html += "  \n";
    html += "  payoffChart.data.datasets.push({\n";
    html += "    label: strategyLabel + ' P&L',\n";
    html += "    data: data.strategyPnL,\n";
    html += "    borderColor: color,\n";
    html += "    backgroundColor: color === '#28a745' ? 'rgba(40, 167, 69, 0.1)' : 'rgba(220, 53, 69, 0.1)',\n";
    html += "    borderWidth: 3,\n";
    html += "    fill: true,\n";
    html += "    tension: 0.1\n";
    html += "  });\n";
    html += "  \n";
    html += "  payoffChart.data.datasets.push({\n";
    html += "    label: 'Zero Line',\n";
    html += "    data: data.prices.map(() => 0),\n";
    html += "    borderColor: '#6c757d',\n";
    html += "    backgroundColor: 'transparent',\n";
    html += "    borderWidth: 1,\n";
    html += "    borderDash: [5, 5],\n";
    html += "    pointRadius: 0,\n";
    html += "    fill: false\n";
    html += "  });\n";
    html += "  \n";
    html += "  if (data.breakevens && data.breakevens.length > 0) {\n";
    html += "    const breakevenPoints = data.breakevens.map(be => ({ x: be, y: 0 }));\n";
    html += "    payoffChart.data.datasets.push({\n";
    html += "      label: 'Breakeven Points',\n";
    html += "      data: breakevenPoints,\n";
    html += "      borderColor: '#ff6b6b',\n";
    html += "      backgroundColor: '#ff6b6b',\n";
    html += "      pointRadius: 8,\n";
    html += "      pointHoverRadius: 10,\n";
    html += "      showLine: false\n";
    html += "    });\n";
    html += "  }\n";
    html += "  \n";
    html += "  payoffChart.update();\n";
    html += "}\n";
    html += "\n";
    html += "function showResults(data) {\n";
    html += "  const isLong = currentPosition === 'long';\n";
    html += "  const breakeven = isLong ? data.breakeven_long : data.breakeven_short;\n";
    html += "  const maxRisk = isLong ? data.maxRisk_long : data.maxRisk_short;\n";
    html += "  const maxProfit = isLong ? data.maxProfit_long : data.maxProfit_short;\n";
    html += "  \n";
    html += "  const moneyness = getMoneyness();\n";
    html += "  const riskReward = maxProfit === 'Unlimited' ? 'Unlimited' : \n";
    html += "                   maxProfit === 'Limited' ? 'Limited' :\n";
    html += "                   (Math.abs(maxProfit) / Math.abs(maxRisk)).toFixed(2) + ':1';\n";
    html += "  \n";
    html += "  const positionType = isLong ? 'Long (Buying)' : 'Short (Selling)';\n";
    html += "  const perspective = isLong ? 'Buyer Perspective' : 'Seller Perspective';\n";
    html += "  \n";
    html += "  const html = `\n";
    html += "    <div class=\"analysis-section\">\n";
    html += "      <h3 style=\"text-align: center; margin-bottom: 20px;\">${positionType} - Risk & Analysis</h3>\n";
    html += "      <div class=\"analysis-grid\">\n";
    html += "        <div class=\"analysis-card\">\n";
    html += "          <h4 style=\"margin-bottom: 10px;\">Key Metrics</h4>\n";
    html += "          <div class=\"analysis-item\">\n";
    html += "            <span>Breakeven Price:</span>\n";
    html += "            <span class=\"analysis-value\">$${breakeven ? breakeven.toFixed(2) : 'N/A'}</span>\n";
    html += "          </div>\n";
    html += "          <div class=\"analysis-item\">\n";
    html += "            <span>Option Premium:</span>\n";
    html += "            <span class=\"analysis-value\">$${data.premium ? data.premium.toFixed(2) : 'N/A'}</span>\n";
    html += "          </div>\n";
    html += "          <div class=\"analysis-item\">\n";
    html += "            <span>Maximum Risk:</span>\n";
    html += "            <span class=\"analysis-value\">${typeof maxRisk === 'string' ? maxRisk : '$' + maxRisk.toFixed(2)}</span>\n";
    html += "          </div>\n";
    html += "          <div class=\"analysis-item\">\n";
    html += "            <span>Maximum Profit:</span>\n";
    html += "            <span class=\"analysis-value\">${typeof maxProfit === 'string' ? maxProfit : '$' + maxProfit.toFixed(2)}</span>\n";
    html += "          </div>\n";
    html += "        </div>\n";
    html += "        \n";
    html += "        <div class=\"analysis-card\">\n";
    html += "          <h4 style=\"margin-bottom: 10px;\">Position Analysis</h4>\n";
    html += "          <div class=\"analysis-item\">\n";
    html += "            <span>Position Type:</span>\n";
    html += "            <span class=\"analysis-value\">${positionType}</span>\n";
    html += "          </div>\n";
    html += "          <div class=\"analysis-item\">\n";
    html += "            <span>Moneyness:</span>\n";
    html += "            <span class=\"analysis-value\">${moneyness}</span>\n";
    html += "          </div>\n";
    html += "          <div class=\"analysis-item\">\n";
    html += "            <span>Risk/Reward:</span>\n";
    html += "            <span class=\"analysis-value\">${riskReward}</span>\n";
    html += "          </div>\n";
    html += "          <div class=\"analysis-item\">\n";
    html += "            <span>Distance to Strike:</span>\n";
    html += "            <span class=\"analysis-value\">${((currentOptionData.spot / currentOptionData.strike - 1) * 100).toFixed(1)}%</span>\n";
    html += "          </div>\n";
    html += "        </div>\n";
    html += "        \n";
    html += "        <div class=\"analysis-card\">\n";
    html += "          <h4 style=\"margin-bottom: 10px;\">${perspective}</h4>\n";
    html += "          <div class=\"analysis-item\">\n";
    html += "            <span>Time Decay:</span>\n";
    html += "            <span class=\"analysis-value\">${isLong ? 'Negative' : 'Positive'}</span>\n";
    html += "          </div>\n";
    html += "          <div class=\"analysis-item\">\n";
    html += "            <span>Volatility Impact:</span>\n";
    html += "            <span class=\"analysis-value\">${isLong ? 'Positive' : 'Negative'}</span>\n";
    html += "          </div>\n";
    html += "          <div class=\"analysis-item\">\n";
    html += "            <span>Assignment Risk:</span>\n";
    html += "            <span class=\"analysis-value\">${isLong ? 'None' : 'Possible'}</span>\n";
    html += "          </div>\n";
    html += "          <div class=\"analysis-item\">\n";
    html += "            <span>Margin Required:</span>\n";
    html += "            <span class=\"analysis-value\">${isLong ? 'Premium Only' : 'Yes'}</span>\n";
    html += "          </div>\n";
    html += "        </div>\n";
    html += "      </div>\n";
    html += "    </div>\n";
    html += "  `;\n";
    html += "  document.getElementById('analysisResults').innerHTML = html;\n";
    html += "}\n";
    html += "\n";
    html += "function showStrategyResults(data) {\n";
    html += "  const strategyName = (strategyPosition === 'long' ? 'Long ' : 'Short ') + strategies[currentStrategy].name;\n";
    html += "  \n";
    html += "  const html = `\n";
    html += "    <div class=\"analysis-section\">\n";
    html += "      <h3 style=\"text-align: center; margin-bottom: 20px;\">${strategyName} - Risk & Analysis</h3>\n";
    html += "      <div class=\"analysis-grid\">\n";
    html += "        <div class=\"analysis-card\">\n";
    html += "          <h4 style=\"margin-bottom: 10px;\">Strategy Metrics</h4>\n";
    html += "          <div class=\"analysis-item\">\n";
    html += "            <span>Net Premium:</span>\n";
    html += "            <span class=\"analysis-value\">$${data.netPremium ? data.netPremium.toFixed(2) : 'N/A'}</span>\n";
    html += "          </div>\n";
    html += "          <div class=\"analysis-item\">\n";
    html += "            <span>Maximum Risk:</span>\n";
    html += "            <span class=\"analysis-value\">${typeof data.maxRisk === 'string' ? data.maxRisk : '$' + data.maxRisk.toFixed(2)}</span>\n";
    html += "          </div>\n";
    html += "          <div class=\"analysis-item\">\n";
    html += "            <span>Maximum Profit:</span>\n";
    html += "            <span class=\"analysis-value\">${typeof data.maxProfit === 'string' ? data.maxProfit : '$' + data.maxProfit.toFixed(2)}</span>\n";
    html += "          </div>\n";
    html += "          <div class=\"analysis-item\">\n";
    html += "            <span>Strategy Type:</span>\n";
    html += "            <span class=\"analysis-value\">${strategyPosition === 'long' ? 'Debit' : 'Credit'}</span>\n";
    html += "          </div>\n";
    html += "        </div>\n";
    html += "        \n";
    html += "        <div class=\"analysis-card\">\n";
    html += "          <h4 style=\"margin-bottom: 10px;\">Breakeven Analysis</h4>\n";
    html += "          ${data.breakevens ? data.breakevens.map((be, index) => `\n";
    html += "          <div class=\"analysis-item\">\n";
    html += "            <span>Breakeven ${index + 1}:</span>\n";
    html += "            <span class=\"analysis-value\">$${be.toFixed(2)}</span>\n";
    html += "          </div>`).join('') : '<p style=\"color: rgba(255,255,255,0.8);\">No breakeven data</p>'}\n";
    html += "          <div class=\"analysis-item\">\n";
    html += "            <span>Profit Range:</span>\n";
    html += "            <span class=\"analysis-value\">${getProfitRange(data)}</span>\n";
    html += "          </div>\n";
    html += "        </div>\n";
    html += "        \n";
    html += "        <div class=\"analysis-card\">\n";
    html += "          <h4 style=\"margin-bottom: 10px;\">Market Outlook</h4>\n";
    html += "          <div class=\"analysis-item\">\n";
    html += "            <span>Volatility Bias:</span>\n";
    html += "            <span class=\"analysis-value\">${getVolatilityBias(currentStrategy, strategyPosition)}</span>\n";
    html += "          </div>\n";
    html += "          <div class=\"analysis-item\">\n";
    html += "            <span>Time Decay:</span>\n";
    html += "            <span class=\"analysis-value\">${getTimeDecayImpact(currentStrategy, strategyPosition)}</span>\n";
    html += "          </div>\n";
    html += "          <div class=\"analysis-item\">\n";
    html += "            <span>Direction Bias:</span>\n";
    html += "            <span class=\"analysis-value\">${getDirectionBias(currentStrategy)}</span>\n";
    html += "          </div>\n";
    html += "          <div class=\"analysis-item\">\n";
    html += "            <span>Complexity:</span>\n";
    html += "            <span class=\"analysis-value\">${getComplexity(currentStrategy)}</span>\n";
    html += "          </div>\n";
    html += "        </div>\n";
    html += "      </div>\n";
    html += "    </div>\n";
    html += "  `;\n";
    html += "  document.getElementById('analysisResults').innerHTML = html;\n";
    html += "}\n";
    html += "\n";
    html += "function getMoneyness() {\n";
    html += "  if (!currentOptionData) return 'N/A';\n";
    html += "  \n";
    html += "  const ratio = currentOptionData.spot / currentOptionData.strike;\n";
    html += "  \n";
    html += "  if (currentOptionData.optionType === 0) {\n";
    html += "    if (ratio > 1.05) return 'ITM (In-the-Money)';\n";
    html += "    if (ratio < 0.95) return 'OTM (Out-of-the-Money)';\n";
    html += "    return 'ATM (At-the-Money)';\n";
    html += "  } else {\n";
    html += "    if (ratio < 0.95) return 'ITM (In-the-Money)';\n";
    html += "    if (ratio > 1.05) return 'OTM (Out-of-the-Money)';\n";
    html += "    return 'ATM (At-the-Money)';\n";
    html += "  }\n";
    html += "}\n";
    html += "\n";
    html += "function getProfitRange(data) {\n";
    html += "  if (!data.breakevens || data.breakevens.length === 0) return 'N/A';\n";
    html += "  if (data.breakevens.length === 1) {\n";
    html += "    return strategyPosition === 'long' ? 'Above $' + data.breakevens[0].toFixed(2) : 'Below $' + data.breakevens[0].toFixed(2);\n";
    html += "  }\n";
    html += "  return '$' + Math.min(...data.breakevens).toFixed(2) + ' - $' + Math.max(...data.breakevens).toFixed(2);\n";
    html += "}\n";
    html += "\n";
    html += "function getVolatilityBias(strategy, position) {\n";
    html += "  const longVolStrategies = ['straddle', 'strangle'];\n";
    html += "  const shortVolStrategies = ['ironcondor'];\n";
    html += "  \n";
    html += "  if (longVolStrategies.includes(strategy)) {\n";
    html += "    return position === 'long' ? 'Long Volatility' : 'Short Volatility';\n";
    html += "  }\n";
    html += "  if (shortVolStrategies.includes(strategy)) {\n";
    html += "    return 'Short Volatility';\n";
    html += "  }\n";
    html += "  return 'Neutral';\n";
    html += "}\n";
    html += "\n";
    html += "function getTimeDecayImpact(strategy, position) {\n";
    html += "  const longStrategies = ['straddle', 'strangle', 'protectiveput'];\n";
    html += "  if (longStrategies.includes(strategy) && position === 'long') {\n";
    html += "    return 'Negative';\n";
    html += "  }\n";
    html += "  return 'Positive';\n";
    html += "}\n";
    html += "\n";
    html += "function getDirectionBias(strategy) {\n";
    html += "  const bullish = ['bullspread', 'coveredcall'];\n";
    html += "  const bearish = ['bearspread', 'protectiveput'];\n";
    html += "  const neutral = ['straddle', 'strangle', 'ironcondor'];\n";
    html += "  \n";
    html += "  if (bullish.includes(strategy)) return 'Bullish';\n";
    html += "  if (bearish.includes(strategy)) return 'Bearish';\n";
    html += "  if (neutral.includes(strategy)) return 'Neutral';\n";
    html += "  return 'Variable';\n";
    html += "}\n";
    html += "\n";
    html += "function getComplexity(strategy) {\n";
    html += "  const simple = ['coveredcall', 'protectiveput'];\n";
    html += "  const moderate = ['straddle', 'strangle', 'bullspread', 'bearspread'];\n";
    html += "  const complex = ['ironcondor'];\n";
    html += "  \n";
    html += "  if (simple.includes(strategy)) return 'Simple';\n";
    html += "  if (moderate.includes(strategy)) return 'Moderate';\n";
    html += "  if (complex.includes(strategy)) return 'Complex';\n";
    html += "  return 'Variable';\n";
    html += "}\n";
    html += "\n";
    html += "function showChart(type) {\n";
    html += "  currentChartType = type;\n";
    html += "  document.querySelectorAll('.chart-btn').forEach(btn => btn.classList.remove('active'));\n";
    html += "  event.target.classList.add('active');\n";
    html += "  \n";
    html += "  if (pricingData) {\n";
    html += "    if (currentStrategy) {\n";
    html += "      updateStrategyChart(pricingData);\n";
    html += "    } else {\n";
    html += "      updateChart(pricingData);\n";
    html += "    }\n";
    html += "  }\n";
    html += "}\n";
    html += "\n";
    html += "document.addEventListener('DOMContentLoaded', initChart);\n";
    html += "</script>\n";
    html += "</body>\n";
    html += "</html>";
    
    return html;
}

std::string WebServer::generateStrategyData(const std::map<std::string, std::string>& params) {
    try {
        double spot = std::stod(params.at("spot"));
        double strike = std::stod(params.at("strike"));
        double rate = std::stod(params.at("rate"));
        double volatility = std::stod(params.at("volatility"));
        double timeToMaturity = std::stod(params.at("timeToMaturity"));
        int optionType = std::stoi(params.at("optionType"));
        int exerciseType = std::stoi(params.at("exerciseType"));
        std::string strategy = params.at("strategy");
        std::string strategyPosition = params.count("strategyPosition") ? params.at("strategyPosition") : "long";
        
        OptionType optType = (optionType == 0) ? OptionType::CALL : OptionType::PUT;
        ExerciseType exType = (exerciseType == 0) ? ExerciseType::EUROPEAN : ExerciseType::AMERICAN;
        
        Option option(spot, strike, rate, volatility, timeToMaturity, optType, exType);
        auto prices = engine_.priceAllMethods(option);
        double premium = prices["BlackScholes"];
        
        double minPrice = strike * 0.5;
        double maxPrice = strike * 1.5;
        int numPoints = 100;
        double priceStep = (maxPrice - minPrice) / numPoints;
        
        std::ostringstream json;
        json << std::fixed << std::setprecision(4);
        json << "{";
        
        json << "\"prices\":[";
        for (int i = 0; i <= numPoints; ++i) {
            double price = minPrice + i * priceStep;
            if (i > 0) json << ",";
            json << price;
        }
        json << "],";
        
        json << "\"strategyPnL\":[";
        
        if (strategy == "straddle") {
            Option callOption(spot, strike, rate, volatility, timeToMaturity, OptionType::CALL, exType);
            Option putOption(spot, strike, rate, volatility, timeToMaturity, OptionType::PUT, exType);
            auto callPrices = engine_.priceAllMethods(callOption);
            auto putPrices = engine_.priceAllMethods(putOption);
            double callPremium = callPrices["BlackScholes"];
            double putPremium = putPrices["BlackScholes"];
            double totalPremium = callPremium + putPremium;
            
            for (int i = 0; i <= numPoints; ++i) {
                double price = minPrice + i * priceStep;
                double callPayoff = std::max(price - strike, 0.0);
                double putPayoff = std::max(strike - price, 0.0);
                double strategyPnL;
                
                if (strategyPosition == "short") {
                    strategyPnL = totalPremium - callPayoff - putPayoff;
                } else {
                    strategyPnL = callPayoff + putPayoff - totalPremium;
                }
                
                if (i > 0) json << ",";
                json << strategyPnL;
            }
            
            json << "],\"netPremium\":" << (strategyPosition == "long" ? -totalPremium : totalPremium);
            if (strategyPosition == "long") {
                json << ",\"maxRisk\":" << totalPremium;
                json << ",\"maxProfit\":\"Unlimited\"";
            } else {
                json << ",\"maxRisk\":\"Unlimited\"";
                json << ",\"maxProfit\":" << totalPremium;
            }
            json << ",\"breakevens\":[" << (strike - totalPremium) << "," << (strike + totalPremium) << "]";
            
        } else if (strategy == "strangle") {
            double callStrike = params.count("strike2") ? std::stod(params.at("strike2")) : strike + 5;
            double putStrike = strike;
            
            Option callOption(spot, callStrike, rate, volatility, timeToMaturity, OptionType::CALL, exType);
            Option putOption(spot, putStrike, rate, volatility, timeToMaturity, OptionType::PUT, exType);
            auto callPrices = engine_.priceAllMethods(callOption);
            auto putPrices = engine_.priceAllMethods(putOption);
            double callPremium = callPrices["BlackScholes"];
            double putPremium = putPrices["BlackScholes"];
            double totalPremium = callPremium + putPremium;
            
            for (int i = 0; i <= numPoints; ++i) {
                double price = minPrice + i * priceStep;
                double callPayoff = std::max(price - callStrike, 0.0);
                double putPayoff = std::max(putStrike - price, 0.0);
                double strategyPnL;
                
                if (strategyPosition == "short") {
                    strategyPnL = totalPremium - callPayoff - putPayoff;
                } else {
                    strategyPnL = callPayoff + putPayoff - totalPremium;
                }
                
                if (i > 0) json << ",";
                json << strategyPnL;
            }
            
            json << "],\"netPremium\":" << (strategyPosition == "long" ? -totalPremium : totalPremium);
            if (strategyPosition == "long") {
                json << ",\"maxRisk\":" << totalPremium;
                json << ",\"maxProfit\":\"Unlimited\"";
            } else {
                json << ",\"maxRisk\":\"Unlimited\"";
                json << ",\"maxProfit\":" << totalPremium;
            }
            json << ",\"breakevens\":[" << (putStrike - totalPremium) << "," << (callStrike + totalPremium) << "]";
            
        } else if (strategy == "bullspread") {
            double longStrike = strike;
            double shortStrike = params.count("strike2") ? std::stod(params.at("strike2")) : strike + 10;
            
            Option longCall(spot, longStrike, rate, volatility, timeToMaturity, OptionType::CALL, exType);
            Option shortCall(spot, shortStrike, rate, volatility, timeToMaturity, OptionType::CALL, exType);
            auto longPrices = engine_.priceAllMethods(longCall);
            auto shortPrices = engine_.priceAllMethods(shortCall);
            double longPremium = longPrices["BlackScholes"];
            double shortPremium = shortPrices["BlackScholes"];
            double netPremium = longPremium - shortPremium;
            
            for (int i = 0; i <= numPoints; ++i) {
                double price = minPrice + i * priceStep;
                double longCallPnL = std::max(price - longStrike, 0.0) - longPremium;
                double shortCallPnL = shortPremium - std::max(price - shortStrike, 0.0);
                double strategyPnL = longCallPnL + shortCallPnL;
                
                if (strategyPosition == "short") {
                    strategyPnL = -strategyPnL;
                }
                
                if (i > 0) json << ",";
                json << strategyPnL;
            }
            
            json << "],\"netPremium\":" << (strategyPosition == "long" ? -netPremium : netPremium);
            if (strategyPosition == "long") {
                json << ",\"maxRisk\":" << netPremium;
                json << ",\"maxProfit\":" << (shortStrike - longStrike - netPremium);
            } else {
                json << ",\"maxRisk\":" << (shortStrike - longStrike - netPremium);
                json << ",\"maxProfit\":" << netPremium;
            }
            json << ",\"breakevens\":[" << (longStrike + netPremium) << "]";
            
        } else if (strategy == "bearspread") {
            double longStrike = params.count("strike2") ? std::stod(params.at("strike2")) : strike + 10;
            double shortStrike = strike;
            
            Option longPut(spot, longStrike, rate, volatility, timeToMaturity, OptionType::PUT, exType);
            Option shortPut(spot, shortStrike, rate, volatility, timeToMaturity, OptionType::PUT, exType);
            auto longPrices = engine_.priceAllMethods(longPut);
            auto shortPrices = engine_.priceAllMethods(shortPut);
            double longPremium = longPrices["BlackScholes"];
            double shortPremium = shortPrices["BlackScholes"];
            double netPremium = longPremium - shortPremium;
            
            for (int i = 0; i <= numPoints; ++i) {
                double price = minPrice + i * priceStep;
                double longPutPnL = std::max(longStrike - price, 0.0) - longPremium;
                double shortPutPnL = shortPremium - std::max(shortStrike - price, 0.0);
                double strategyPnL = longPutPnL + shortPutPnL;
                
                if (strategyPosition == "short") {
                    strategyPnL = -strategyPnL;
                }
                
                if (i > 0) json << ",";
                json << strategyPnL;
            }
            
            json << "],\"netPremium\":" << (strategyPosition == "long" ? -netPremium : netPremium);
            if (strategyPosition == "long") {
                json << ",\"maxRisk\":" << netPremium;
                json << ",\"maxProfit\":" << (longStrike - shortStrike - netPremium);
            } else {
                json << ",\"maxRisk\":" << (longStrike - shortStrike - netPremium);
                json << ",\"maxProfit\":" << netPremium;
            }
            json << ",\"breakevens\":[" << (longStrike - netPremium) << "]";
            
        } else if (strategy == "ironcondor") {
            double putStrike1 = strike - 10;
            double putStrike2 = strike - 5;
            double callStrike1 = strike + 5;
            double callStrike2 = params.count("strike2") ? std::stod(params.at("strike2")) : strike + 10;
            
            Option shortPut1(spot, putStrike1, rate, volatility, timeToMaturity, OptionType::PUT, exType);
            Option longPut2(spot, putStrike2, rate, volatility, timeToMaturity, OptionType::PUT, exType);
            Option longCall1(spot, callStrike1, rate, volatility, timeToMaturity, OptionType::CALL, exType);
            Option shortCall2(spot, callStrike2, rate, volatility, timeToMaturity, OptionType::CALL, exType);
            
            auto putPrices1 = engine_.priceAllMethods(shortPut1);
            auto putPrices2 = engine_.priceAllMethods(longPut2);
            auto callPrices1 = engine_.priceAllMethods(longCall1);
            auto callPrices2 = engine_.priceAllMethods(shortCall2);
            
            double netCredit = putPrices1["BlackScholes"] - putPrices2["BlackScholes"] + 
                              callPrices2["BlackScholes"] - callPrices1["BlackScholes"];
            
            for (int i = 0; i <= numPoints; ++i) {
                double price = minPrice + i * priceStep;
                
                double putSpreadPnL = putPrices1["BlackScholes"] - std::max(putStrike1 - price, 0.0) -
                                     putPrices2["BlackScholes"] + std::max(putStrike2 - price, 0.0);
                double callSpreadPnL = callPrices2["BlackScholes"] - std::max(price - callStrike2, 0.0) -
                                      callPrices1["BlackScholes"] + std::max(price - callStrike1, 0.0);
                
                double strategyPnL = putSpreadPnL + callSpreadPnL;
                
                if (strategyPosition == "short") {
                    strategyPnL = -strategyPnL;
                }
                
                if (i > 0) json << ",";
                json << strategyPnL;
            }
            
            json << "],\"netPremium\":" << (strategyPosition == "long" ? -netCredit : netCredit);
            if (strategyPosition == "long") {
                json << ",\"maxRisk\":" << (putStrike2 - putStrike1 - netCredit);
                json << ",\"maxProfit\":" << netCredit;
            } else {
                json << ",\"maxRisk\":\"Unlimited\"";
                json << ",\"maxProfit\":\"Unlimited\"";
            }
            json << ",\"breakevens\":[" << (putStrike2 - netCredit) << "," << (callStrike1 + netCredit) << "]";
            
        } else if (strategy == "coveredcall") {
            double callPremium = premium;
            
            for (int i = 0; i <= numPoints; ++i) {
                double price = minPrice + i * priceStep;
                double stockPnL = price - spot;
                double callPnL = callPremium - std::max(price - strike, 0.0);
                double strategyPnL = stockPnL + callPnL;
                
                if (i > 0) json << ",";
                json << strategyPnL;
            }
            
            json << "],\"netPremium\":" << callPremium;
            json << ",\"maxRisk\":\"Stock decline\"";
            json << ",\"maxProfit\":" << (strike - spot + callPremium);
            json << ",\"breakevens\":[" << (spot - callPremium) << "]";
            
        } else if (strategy == "protectiveput") {
            double putPremium = premium;
            
            for (int i = 0; i <= numPoints; ++i) {
                double price = minPrice + i * priceStep;
                double stockPnL = price - spot;
                double putPnL = std::max(strike - price, 0.0) - putPremium;
                double strategyPnL = stockPnL + putPnL;
                
                if (i > 0) json << ",";
                json << strategyPnL;
            }
            
            json << "],\"netPremium\":" << -putPremium;
            json << ",\"maxRisk\":" << (spot - strike + putPremium);
            json << ",\"maxProfit\":\"Unlimited\"";
            json << ",\"breakevens\":[" << (spot + putPremium) << "]";
        }
        
        json << ",\"probProfit\":65.5";
        json << "}";
        
        return json.str();
        
    } catch (const std::exception& e) {
        return "{\"error\":\"" + std::string(e.what()) + "\"}";
    }
}

std::string WebServer::generatePayoffData(const std::map<std::string, std::string>& params) {
    try {
        double spot = std::stod(params.at("spot"));
        double strike = std::stod(params.at("strike"));
        double rate = std::stod(params.at("rate"));
        double volatility = std::stod(params.at("volatility"));
        double timeToMaturity = std::stod(params.at("timeToMaturity"));
        int optionType = std::stoi(params.at("optionType"));
        int exerciseType = std::stoi(params.at("exerciseType"));
        
        OptionType optType = (optionType == 0) ? OptionType::CALL : OptionType::PUT;
        ExerciseType exType = (exerciseType == 0) ? ExerciseType::EUROPEAN : ExerciseType::AMERICAN;
        
        Option option(spot, strike, rate, volatility, timeToMaturity, optType, exType);
        auto prices = engine_.priceAllMethods(option);
        double premium = prices["BlackScholes"];
        
        double minPrice = strike * 0.5;
        double maxPrice = strike * 1.5;
        int numPoints = 100;
        double priceStep = (maxPrice - minPrice) / numPoints;
        
        std::ostringstream json;
        json << std::fixed << std::setprecision(4);
        json << "{";
        
        json << "\"prices\":[";
        for (int i = 0; i <= numPoints; ++i) {
            double price = minPrice + i * priceStep;
            if (i > 0) json << ",";
            json << price;
        }
        json << "],";
        
        json << "\"payoffs\":[";
        for (int i = 0; i <= numPoints; ++i) {
            double price = minPrice + i * priceStep;
            double payoff = 0;
            
            if (optionType == 0) {
                payoff = std::max(price - strike, 0.0);
            } else {
                payoff = std::max(strike - price, 0.0);
            }
            
            if (i > 0) json << ",";
            json << payoff;
        }
        json << "],";
        
        json << "\"pnl_long\":[";
        for (int i = 0; i <= numPoints; ++i) {
            double price = minPrice + i * priceStep;
            double payoff = 0;
            
            if (optionType == 0) {
                payoff = std::max(price - strike, 0.0);
            } else {
                payoff = std::max(strike - price, 0.0);
            }
            
            double pnl = payoff - premium;
            
            if (i > 0) json << ",";
            json << pnl;
        }
        json << "],";
        
        json << "\"pnl_short\":[";
        for (int i = 0; i <= numPoints; ++i) {
            double price = minPrice + i * priceStep;
            double payoff = 0;
            
            if (optionType == 0) {
                payoff = std::max(price - strike, 0.0);
            } else {
                payoff = std::max(strike - price, 0.0);
            }
            
            double pnl = premium - payoff;
            
            if (i > 0) json << ",";
            json << pnl;
        }
        json << "],";
        
        double breakeven_long = 0;
        double breakeven_short = 0;
        
        if (optionType == 0) {
            breakeven_long = strike + premium;
            breakeven_short = strike + premium;
        } else {
            breakeven_long = strike - premium;
            breakeven_short = strike - premium;
        }
        
        json << "\"breakeven_long\":" << breakeven_long << ",";
        json << "\"breakeven_short\":" << breakeven_short << ",";
        json << "\"maxRisk_long\":" << premium << ",";
        json << "\"maxRisk_short\":\"Unlimited\",";
        json << "\"premium\":" << premium << ",";
        
        if (optionType == 0) {
            json << "\"maxProfit_long\":\"Unlimited\",";
            json << "\"maxProfit_short\":" << premium;
        } else {
            double maxProfit_long = std::max(0.0, strike - premium);
            json << "\"maxProfit_long\":" << maxProfit_long << ",";
            json << "\"maxProfit_short\":" << premium;
        }
        
        json << "}";
        return json.str();
        
    } catch (const std::exception& e) {
        return "{\"error\":\"" + std::string(e.what()) + "\"}";
    }
}

std::string WebServer::handlePricingRequest(const std::map<std::string, std::string>& params) {
    try {
        double spot = std::stod(params.at("spot"));
        double strike = std::stod(params.at("strike"));
        double rate = std::stod(params.at("rate"));
        double volatility = std::stod(params.at("volatility"));
        double timeToMaturity = std::stod(params.at("timeToMaturity"));
        int optionType = std::stoi(params.at("optionType"));
        int exerciseType = std::stoi(params.at("exerciseType"));
        
        OptionType optType = (optionType == 0) ? OptionType::CALL : OptionType::PUT;
        ExerciseType exType = (exerciseType == 0) ? ExerciseType::EUROPEAN : ExerciseType::AMERICAN;
        
        Option option(spot, strike, rate, volatility, timeToMaturity, optType, exType);
        auto prices = engine_.priceAllMethods(option);
        
        std::ostringstream json;
        json << std::fixed << std::setprecision(6);
        json << "{\"prices\":{";
        
        bool first = true;
        for (auto it = prices.begin(); it != prices.end(); ++it) {
            if (!first) json << ",";
            json << "\"" << it->first << "\":" << it->second;
            first = false;
        }
        json << "}";
        
        if (exerciseType == 0) {
            auto greeks = engine_.calculateGreeks(option);
            json << ",\"greeks\":{";
            first = true;
            for (auto it = greeks.begin(); it != greeks.end(); ++it) {
                if (!first) json << ",";
                json << "\"" << it->first << "\":" << it->second;
                first = false;
            }
            json << "}";
        }
        
        json << "}";
        return json.str();
        
    } catch (const std::exception& e) {
        return "{\"error\":\"" + std::string(e.what()) + "\"}";
    }
}

std::map<std::string, std::string> WebServer::parseQueryString(const std::string& query) {
    std::map<std::string, std::string> params;
    std::istringstream iss(query);
    std::string pair;
    
    while (std::getline(iss, pair, '&')) {
        size_t eq_pos = pair.find('=');
        if (eq_pos != std::string::npos) {
            std::string key = pair.substr(0, eq_pos);
            std::string value = pair.substr(eq_pos + 1);
            params[key] = value;
        }
    }
    
    return params;
}

void WebServer::sendHTTPResponse(int client_socket, const std::string& content, const std::string& contentType) {
    std::ostringstream response;
    response << "HTTP/1.1 200 OK\r\n";
    response << "Content-Type: " << contentType << "\r\n";
    response << "Content-Length: " << content.length() << "\r\n";
    response << "Access-Control-Allow-Origin: *\r\n";
    response << "\r\n";
    response << content;
    
    send(client_socket, response.str().c_str(), response.str().length(), 0);
}

void WebServer::stop() {
    running_ = false;
}

