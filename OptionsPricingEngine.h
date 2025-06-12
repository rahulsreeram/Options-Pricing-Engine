// OptionsPricingEngine.h
#ifndef OPTIONS_PRICING_ENGINE_H
#define OPTIONS_PRICING_ENGINE_H

#include "Option.h"
#include "BlackScholesEngine.h"
#include "BinomialEngine.h"
#include "MonteCarloEngine.h"
#include <memory>
#include <map>
#include <functional>

class OptionsPricingEngine {
public:
    OptionsPricingEngine();
    
    double price(const Option& option, const std::string& method = "BlackScholes");
    std::map<std::string, double> priceAllMethods(const Option& option);
    std::map<std::string, double> calculateGreeks(const Option& option);
    
    void setBinomialSteps(int steps);
    void setMonteCarloSimulations(int simulations);

private:
    std::unique_ptr<BlackScholesEngine> blackScholesEngine_;
    std::unique_ptr<BinomialEngine> binomialEngine_;
    std::unique_ptr<MonteCarloEngine> monteCarloEngine_;
    
    // Use function pointers instead of storing unique_ptr references
    std::map<std::string, std::function<double(const Option&)>> pricingFunctions_;
};

#endif
