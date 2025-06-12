#include "OptionsPricingEngine.h"
#include <iostream>
#include <iomanip>

int main() {
    // Create option parameters
    double spot = 100.0;
    double strike = 105.0;
    double rate = 0.05;
    double volatility = 0.2;
    double timeToMaturity = 0.25; // 3 months
    
    // Create European call option
    Option europeanCall(spot, strike, rate, volatility, timeToMaturity, 
                       OptionType::CALL, ExerciseType::EUROPEAN);
    
    // Create American put option
    Option americanPut(spot, strike, rate, volatility, timeToMaturity, 
                      OptionType::PUT, ExerciseType::AMERICAN);
    
    // Initialize pricing engine
    OptionsPricingEngine engine;
    engine.setBinomialSteps(1000);
    engine.setMonteCarloSimulations(100000);
    
    std::cout << std::fixed << std::setprecision(4);
    
    // Price European call using all methods
    std::cout << "European Call Option Pricing:\n";
    std::cout << "Spot: $" << spot << ", Strike: $" << strike << ", Vol: " << volatility*100 << "%\n\n";
    
    auto europeanResults = engine.priceAllMethods(europeanCall);
    for (const auto& result : europeanResults) {
        std::cout << result.first << ": $" << result.second << "\n";
    }
    
    // Calculate Greeks for European call
    std::cout << "\nGreeks for European Call:\n";
    auto greeks = engine.calculateGreeks(europeanCall);
    for (const auto& greekPair : greeks) {
        std::cout << greekPair.first << ": " << greekPair.second << "\n";
    }
    
    // Price American put (only Binomial supports American options)
    std::cout << "\nAmerican Put Option Pricing:\n";
    try {
        double americanPrice = engine.price(americanPut, "Binomial");
        std::cout << "Binomial: $" << americanPrice << "\n";
        
        // Compare with European put
        Option europeanPut(spot, strike, rate, volatility, timeToMaturity, 
                          OptionType::PUT, ExerciseType::EUROPEAN);
        double europeanPutPrice = engine.price(europeanPut, "BlackScholes");
        std::cout << "European Put (BS): $" << europeanPutPrice << "\n";
        std::cout << "Early Exercise Premium: $" << (americanPrice - europeanPutPrice) << "\n";
        
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << "\n";
    }
    
    return 0;
}
