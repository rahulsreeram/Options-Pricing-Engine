// BinomialEngine.h
#ifndef BINOMIAL_ENGINE_H
#define BINOMIAL_ENGINE_H

#include "PricingEngine.h"
#include <vector>

class BinomialEngine : public PricingEngine {
public:
    BinomialEngine(int steps = 100) : steps_(steps) {}
    
    double price(const Option& option) override;
    std::string getMethodName() const override { return "Binomial Tree"; }
    
    void setSteps(int steps) { steps_ = steps; }

private:
    int steps_;
    
    struct TreeParameters {
        double u, d, p;
        double dt;
    };
    
    TreeParameters calculateTreeParameters(const Option& option);
};

#endif
