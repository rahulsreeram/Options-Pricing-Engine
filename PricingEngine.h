// PricingEngine.h
#ifndef PRICING_ENGINE_H
#define PRICING_ENGINE_H

#include "Option.h"

class PricingEngine {
public:
    virtual ~PricingEngine() = default;
    virtual double price(const Option& option) = 0;
    virtual std::string getMethodName() const = 0;
};

#endif
