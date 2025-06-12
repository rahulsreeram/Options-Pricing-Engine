// BlackScholesEngine.h
#ifndef BLACK_SCHOLES_ENGINE_H
#define BLACK_SCHOLES_ENGINE_H

#include "PricingEngine.h"
#include <cmath>

class BlackScholesEngine : public PricingEngine {
public:
    double price(const Option& option) override;
    std::string getMethodName() const override { return "Black-Scholes"; }
    
    // Greeks calculation
    double delta(const Option& option);
    double gamma(const Option& option);
    double theta(const Option& option);
    double vega(const Option& option);
    double rho(const Option& option);

private:
    double cumulativeNormalDistribution(double x);
    double normalProbabilityDensity(double x);
    std::pair<double, double> calculateD1D2(const Option& option);
};

#endif
