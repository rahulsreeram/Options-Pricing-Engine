#include "BlackScholesEngine.h"
#include <cmath>
#include <stdexcept>

double BlackScholesEngine::price(const Option& option) {
    if (option.getExerciseType() == ExerciseType::AMERICAN) {
        throw std::invalid_argument("Black-Scholes only supports European options");
    }
    
    // C++14 compatible - no structured binding
    std::pair<double, double> d_values = calculateD1D2(option);
    double d1 = d_values.first;
    double d2 = d_values.second;
    
    if (option.getOptionType() == OptionType::CALL) {
        return option.getSpot() * cumulativeNormalDistribution(d1) - 
               option.getStrike() * std::exp(-option.getRate() * option.getTimeToMaturity()) * 
               cumulativeNormalDistribution(d2);
    } else {
        return option.getStrike() * std::exp(-option.getRate() * option.getTimeToMaturity()) * 
               cumulativeNormalDistribution(-d2) - 
               option.getSpot() * cumulativeNormalDistribution(-d1);
    }
}

std::pair<double, double> BlackScholesEngine::calculateD1D2(const Option& option) {
    double S = option.getSpot();
    double K = option.getStrike();
    double r = option.getRate();
    double sigma = option.getVolatility();
    double T = option.getTimeToMaturity();
    
    double d1 = (std::log(S / K) + (r + 0.5 * sigma * sigma) * T) / (sigma * std::sqrt(T));
    double d2 = d1 - sigma * std::sqrt(T);
    
    return std::make_pair(d1, d2);
}

double BlackScholesEngine::cumulativeNormalDistribution(double x) {
    const double a1 =  0.254829592;
    const double a2 = -0.284496736;
    const double a3 =  1.421413741;
    const double a4 = -1.453152027;
    const double a5 =  1.061405429;
    const double p  =  0.3275911;
    
    int sign = (x >= 0) ? 1 : -1;
    x = std::abs(x) / std::sqrt(2.0);
    
    double t = 1.0 / (1.0 + p * x);
    double y = 1.0 - (((((a5 * t + a4) * t) + a3) * t + a2) * t + a1) * t * std::exp(-x * x);
    
    return 0.5 * (1.0 + sign * y);
}

double BlackScholesEngine::normalProbabilityDensity(double x) {
    return std::exp(-0.5 * x * x) / std::sqrt(2.0 * M_PI);
}

double BlackScholesEngine::delta(const Option& option) {
    // C++14 compatible - no structured binding
    std::pair<double, double> d_values = calculateD1D2(option);
    double d1 = d_values.first;
    
    if (option.getOptionType() == OptionType::CALL) {
        return cumulativeNormalDistribution(d1);
    } else {
        return cumulativeNormalDistribution(d1) - 1.0;
    }
}

double BlackScholesEngine::gamma(const Option& option) {
    // C++14 compatible - no structured binding
    std::pair<double, double> d_values = calculateD1D2(option);
    double d1 = d_values.first;
    
    return normalProbabilityDensity(d1) / 
           (option.getSpot() * option.getVolatility() * std::sqrt(option.getTimeToMaturity()));
}

double BlackScholesEngine::theta(const Option& option) {
    // C++14 compatible - no structured binding
    std::pair<double, double> d_values = calculateD1D2(option);
    double d1 = d_values.first;
    double d2 = d_values.second;
    
    double S = option.getSpot();
    double K = option.getStrike();
    double r = option.getRate();
    double sigma = option.getVolatility();
    double T = option.getTimeToMaturity();
    
    double term1 = -(S * normalProbabilityDensity(d1) * sigma) / (2 * std::sqrt(T));
    
    if (option.getOptionType() == OptionType::CALL) {
        double term2 = r * K * std::exp(-r * T) * cumulativeNormalDistribution(d2);
        return (term1 - term2) / 365.0;
    } else {
        double term2 = r * K * std::exp(-r * T) * cumulativeNormalDistribution(-d2);
        return (term1 + term2) / 365.0;
    }
}

double BlackScholesEngine::vega(const Option& option) {
    // C++14 compatible - no structured binding
    std::pair<double, double> d_values = calculateD1D2(option);
    double d1 = d_values.first;
    
    double S = option.getSpot();
    double T = option.getTimeToMaturity();
    
    return S * std::sqrt(T) * normalProbabilityDensity(d1) / 100.0;
}

double BlackScholesEngine::rho(const Option& option) {
    // C++14 compatible - no structured binding
    std::pair<double, double> d_values = calculateD1D2(option);
    double d2 = d_values.second;
    
    double K = option.getStrike();
    double r = option.getRate();
    double T = option.getTimeToMaturity();
    
    if (option.getOptionType() == OptionType::CALL) {
        return K * T * std::exp(-r * T) * cumulativeNormalDistribution(d2) / 100.0;
    } else {
        return -K * T * std::exp(-r * T) * cumulativeNormalDistribution(-d2) / 100.0;
    }
}
