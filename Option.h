#ifndef OPTION_H
#define OPTION_H

#include <memory>
#include <vector>
#include <functional>
#include <algorithm>
#include <cmath>
#include <string>

enum class OptionType { CALL, PUT };
enum class ExerciseType { EUROPEAN, AMERICAN };
enum class PositionType { LONG, SHORT };

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
    
    // Payoff function (intrinsic value)
    double payoff(double spotPrice) const {
        if (optionType_ == OptionType::CALL) {
            return std::max(spotPrice - K_, 0.0);
        } else {
            return std::max(K_ - spotPrice, 0.0);
        }
    }
    
    // P&L calculation for different positions
    double calculatePnL(double spotPrice, double premium, PositionType position) const {
        double intrinsicValue = payoff(spotPrice);
        if (position == PositionType::LONG) {
            return intrinsicValue - premium;  // Buyer's P&L
        } else {
            return premium - intrinsicValue;  // Seller's P&L
        }
    }

private:
    double S0_, K_, r_, sigma_, T_;
    OptionType optionType_;
    ExerciseType exerciseType_;
};

// Strategy class for combinations
class OptionStrategy {
public:
    struct OptionLeg {
        Option option;
        PositionType position;
        double premium;
        int quantity;
        
        OptionLeg(const Option& opt, PositionType pos, double prem, int qty = 1)
            : option(opt), position(pos), premium(prem), quantity(qty) {}
    };
    
    std::vector<OptionLeg> legs;
    
    void addLeg(const Option& option, PositionType position, double premium, int quantity = 1) {
        legs.emplace_back(option, position, premium, quantity);
    }
    
    double calculateStrategyPnL(double spotPrice) const {
        double totalPnL = 0.0;
        for (const auto& leg : legs) {
            double legPnL = leg.option.calculatePnL(spotPrice, leg.premium, leg.position);
            totalPnL += legPnL * leg.quantity;
        }
        return totalPnL;
    }
    
    double getNetPremium() const {
        double netPremium = 0.0;
        for (const auto& leg : legs) {
            if (leg.position == PositionType::LONG) {
                netPremium -= leg.premium * leg.quantity;  // Pay premium
            } else {
                netPremium += leg.premium * leg.quantity;  // Receive premium
            }
        }
        return netPremium;
    }
    
    std::string getStrategyName() const {
        if (legs.size() == 1) {
            const auto& leg = legs[0];
            std::string posStr = (leg.position == PositionType::LONG) ? "Long" : "Short";
            std::string typeStr = (leg.option.getOptionType() == OptionType::CALL) ? "Call" : "Put";
            return posStr + " " + typeStr;
        } else if (legs.size() == 2) {
            // Detect common strategies
            if (isStraddle()) return "Straddle";
            if (isStrangle()) return "Strangle";
            if (isSpread()) return "Spread";
        }
        return "Custom Strategy";
    }
    
private:
    bool isStraddle() const {
        if (legs.size() != 2) return false;
        return (legs[0].option.getStrike() == legs[1].option.getStrike() &&
                legs[0].option.getOptionType() != legs[1].option.getOptionType() &&
                legs[0].position == legs[1].position);
    }
    
    bool isStrangle() const {
        if (legs.size() != 2) return false;
        return (legs[0].option.getStrike() != legs[1].option.getStrike() &&
                legs[0].option.getOptionType() != legs[1].option.getOptionType() &&
                legs[0].position == legs[1].position);
    }
    
    bool isSpread() const {
        if (legs.size() != 2) return false;
        return (legs[0].option.getOptionType() == legs[1].option.getOptionType() &&
                legs[0].position != legs[1].position);
    }
};

#endif
