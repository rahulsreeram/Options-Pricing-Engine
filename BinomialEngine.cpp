#include "BinomialEngine.h"
#include <cmath>
#include <algorithm>

double BinomialEngine::price(const Option& option) {
    TreeParameters params = calculateTreeParameters(option);
    
    // Initialize option values at maturity
    std::vector<double> optionValues(steps_ + 1);
    
    // Calculate option values at expiration
    for (int i = 0; i <= steps_; ++i) {
        double spotAtExpiry = option.getSpot() * std::pow(params.u, i) * std::pow(params.d, steps_ - i);
        optionValues[i] = option.payoff(spotAtExpiry);
    }
    
    // Backward induction
    for (int step = steps_ - 1; step >= 0; --step) {
        for (int i = 0; i <= step; ++i) {
            // Calculate continuation value
            double continuationValue = std::exp(-option.getRate() * params.dt) * 
                                     (params.p * optionValues[i + 1] + (1 - params.p) * optionValues[i]);
            
            if (option.getExerciseType() == ExerciseType::AMERICAN) {
                // Calculate intrinsic value for early exercise
                double spotPrice = option.getSpot() * std::pow(params.u, i) * std::pow(params.d, step - i);
                double intrinsicValue = option.payoff(spotPrice);
                optionValues[i] = std::max(continuationValue, intrinsicValue);
            } else {
                optionValues[i] = continuationValue;
            }
        }
    }
    
    return optionValues[0];
}

BinomialEngine::TreeParameters BinomialEngine::calculateTreeParameters(const Option& option) {
    TreeParameters params;
    params.dt = option.getTimeToMaturity() / steps_;
    
    // Cox-Ross-Rubinstein parameterization
    params.u = std::exp(option.getVolatility() * std::sqrt(params.dt));
    params.d = 1.0 / params.u;
    params.p = (std::exp(option.getRate() * params.dt) - params.d) / (params.u - params.d);
    
    return params;
}
