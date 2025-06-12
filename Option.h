// Option.h
#ifndef OPTION_H
#define OPTION_H

#include <memory>
#include <vector>
#include <functional>

enum class OptionType { CALL, PUT };
enum class ExerciseType { EUROPEAN, AMERICAN };

class Option {
public:
    Option(double spot, double strike, double rate, double volatility, 
           double timeToMaturity, OptionType type, ExerciseType exercise)
        : S0_(spot), K_(strike), r_(rate), sigma_(volatility), 
          T_(timeToMaturity), optionType_(type), exerciseType_(exercise) {}
    
    // Getters
    double getSpot() const { return S0_; }
    double getStrike() const { return K_; }
    double getRate() const { return r_; }
    double getVolatility() const { return sigma_; }
    double getTimeToMaturity() const { return T_; }
    OptionType getOptionType() const { return optionType_; }
    ExerciseType getExerciseType() const { return exerciseType_; }
    
    // Payoff function
    double payoff(double spotPrice) const {
        if (optionType_ == OptionType::CALL) {
            return std::max(spotPrice - K_, 0.0);
        } else {
            return std::max(K_ - spotPrice, 0.0);
        }
    }

private:
    double S0_, K_, r_, sigma_, T_;
    OptionType optionType_;
    ExerciseType exerciseType_;
};

#endif
