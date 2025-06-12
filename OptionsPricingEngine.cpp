#include "OptionsPricingEngine.h"
#include <stdexcept>
#include <limits>

OptionsPricingEngine::OptionsPricingEngine() 
    : blackScholesEngine_(std::make_unique<BlackScholesEngine>()),
      binomialEngine_(std::make_unique<BinomialEngine>()),
      monteCarloEngine_(std::make_unique<MonteCarloEngine>()) {
    
    pricingFunctions_["BlackScholes"] = [this](const Option& option) {
        return blackScholesEngine_->price(option);
    };
    
    pricingFunctions_["Binomial"] = [this](const Option& option) {
        return binomialEngine_->price(option);
    };
    
    pricingFunctions_["MonteCarlo"] = [this](const Option& option) {
        return monteCarloEngine_->price(option);
    };
}

double OptionsPricingEngine::price(const Option& option, const std::string& method) {
    auto it = pricingFunctions_.find(method);
    if (it == pricingFunctions_.end()) {
        throw std::invalid_argument("Unknown pricing method: " + method);
    }
    
    return it->second(option);
}

std::map<std::string, double> OptionsPricingEngine::priceAllMethods(const Option& option) {
    std::map<std::string, double> results;
    
    // C++14 compatible - replace structured binding
    for (auto it = pricingFunctions_.begin(); it != pricingFunctions_.end(); ++it) {
        const std::string& name = it->first;
        const std::function<double(const Option&)>& pricingFunc = it->second;
        try {
            results[name] = pricingFunc(option);
        } catch (const std::exception& e) {
            results[name] = std::numeric_limits<double>::quiet_NaN();
        }
    }
    
    return results;
}

std::map<std::string, double> OptionsPricingEngine::calculateGreeks(const Option& option) {
    std::map<std::string, double> greeks;
    
    greeks["Delta"] = blackScholesEngine_->delta(option);
    greeks["Gamma"] = blackScholesEngine_->gamma(option);
    greeks["Theta"] = blackScholesEngine_->theta(option);
    greeks["Vega"] = blackScholesEngine_->vega(option);
    greeks["Rho"] = blackScholesEngine_->rho(option);
    
    return greeks;
}

void OptionsPricingEngine::setBinomialSteps(int steps) {
    binomialEngine_->setSteps(steps);
}

void OptionsPricingEngine::setMonteCarloSimulations(int simulations) {
    monteCarloEngine_->setNumSimulations(simulations);
}
