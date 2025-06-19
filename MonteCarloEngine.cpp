#include "MonteCarloEngine.h"
#include <cmath>
#include <numeric>
#include <stdexcept>

double MonteCarloEngine::price(const Option& option) {
    if (option.getExerciseType() == ExerciseType::AMERICAN) {
        throw std::invalid_argument("Basic Monte Carlo doesn't support American options");
    }
    
    std::vector<double> payoffs;
    payoffs.reserve(numSimulations_);
    
    for (int i = 0; i < numSimulations_; ++i) {
        double randomNormal = normalDist_(generator_);
        double finalSpotPrice = simulateSpotPrice(option, randomNormal);
        payoffs.push_back(option.payoff(finalSpotPrice));
    }
    
    double averagePayoff = std::accumulate(payoffs.begin(), payoffs.end(), 0.0) / numSimulations_;
    return std::exp(-option.getRate() * option.getTimeToMaturity()) * averagePayoff;
}

double MonteCarloEngine::simulateSpotPrice(const Option& option, double randomNormal) {
    double drift = (option.getRate() - 0.5 * option.getVolatility() * option.getVolatility()) * 
                   option.getTimeToMaturity();
    double diffusion = option.getVolatility() * std::sqrt(option.getTimeToMaturity()) * randomNormal;
    
    return option.getSpot() * std::exp(drift + diffusion);
}

double MonteCarloEngine::priceWithAntithetic(const Option& option) {
    std::vector<double> payoffs;
    payoffs.reserve(numSimulations_);
    
    for (int i = 0; i < numSimulations_ / 2; ++i) {
        double randomNormal = normalDist_(generator_);
        
        // Original path
        double finalSpotPrice1 = simulateSpotPrice(option, randomNormal);
        payoffs.push_back(option.payoff(finalSpotPrice1));
        
        // Antithetic path
        double finalSpotPrice2 = simulateSpotPrice(option, -randomNormal);
        payoffs.push_back(option.payoff(finalSpotPrice2));
    }
    
    double averagePayoff = std::accumulate(payoffs.begin(), payoffs.end(), 0.0) / payoffs.size();
    return std::exp(-option.getRate() * option.getTimeToMaturity()) * averagePayoff;
}

double MonteCarloEngine::priceWithControlVariate(const Option& option) {
    // Simple control variate implementation using geometric average
    std::vector<double> payoffs;
    std::vector<double> controlVariates;
    payoffs.reserve(numSimulations_);
    controlVariates.reserve(numSimulations_);
    
    for (int i = 0; i < numSimulations_; ++i) {
        double randomNormal = normalDist_(generator_);
        double finalSpotPrice = simulateSpotPrice(option, randomNormal);
        
        payoffs.push_back(option.payoff(finalSpotPrice));
        controlVariates.push_back(finalSpotPrice - option.getSpot() * 
                                 std::exp(option.getRate() * option.getTimeToMaturity()));
    }
    
    // Calculate control variate coefficient
    double payoffMean = std::accumulate(payoffs.begin(), payoffs.end(), 0.0) / numSimulations_;
    double cvMean = std::accumulate(controlVariates.begin(), controlVariates.end(), 0.0) / numSimulations_;
    
    double covariance = 0.0;
    double cvVariance = 0.0;
    
    for (int i = 0; i < numSimulations_; ++i) {
        covariance += (payoffs[i] - payoffMean) * (controlVariates[i] - cvMean);
        cvVariance += (controlVariates[i] - cvMean) * (controlVariates[i] - cvMean);
    }
    
    double beta = covariance / cvVariance;
    
    // Apply control variate
    double adjustedPayoff = payoffMean - beta * cvMean;
    
    return std::exp(-option.getRate() * option.getTimeToMaturity()) * adjustedPayoff;
}

std::vector<double> MonteCarloEngine::generatePath(const Option& option, int numSteps) {
    std::vector<double> path;
    path.reserve(numSteps + 1);
    
    double dt = option.getTimeToMaturity() / numSteps;
    double currentPrice = option.getSpot();
    path.push_back(currentPrice);
    
    for (int i = 0; i < numSteps; ++i) {
        double randomNormal = normalDist_(generator_);
        double drift = (option.getRate() - 0.5 * option.getVolatility() * option.getVolatility()) * dt;
        double diffusion = option.getVolatility() * std::sqrt(dt) * randomNormal;
        
        currentPrice *= std::exp(drift + diffusion);
        path.push_back(currentPrice);
    }
    
    return path;
}
