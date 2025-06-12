// MonteCarloEngine.h
#ifndef MONTE_CARLO_ENGINE_H
#define MONTE_CARLO_ENGINE_H

#include "PricingEngine.h"
#include <random>
#include <vector>

class MonteCarloEngine : public PricingEngine {
public:
    MonteCarloEngine(int numSimulations = 100000, int seed = 42) 
        : numSimulations_(numSimulations), generator_(seed) {}
    
    double price(const Option& option) override;
    std::string getMethodName() const override { return "Monte Carlo"; }
    
    void setNumSimulations(int numSimulations) { numSimulations_ = numSimulations; }
    
    // Advanced features
    double priceWithAntithetic(const Option& option);
    double priceWithControlVariate(const Option& option);

private:
    int numSimulations_;
    std::mt19937 generator_;
    std::normal_distribution<double> normalDist_{0.0, 1.0};
    
    double simulateSpotPrice(const Option& option, double randomNormal);
    std::vector<double> generatePath(const Option& option, int numSteps = 252);
};

#endif
