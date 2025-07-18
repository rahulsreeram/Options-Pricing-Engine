#include "OptionsPricingEngine.h"
#include <iostream>
#include <iomanip>
#include <string>
#include <limits>

class InteractiveOptionsPricer {
private:
    OptionsPricingEngine engine;
    
    double getValidInput(const std::string& prompt, double minValue = 0.0) {
        double value;
        while (true) {
            std::cout << prompt;
            if (std::cin >> value && value > minValue) {
                return value;
            }
            std::cout << "Invalid input. Please enter a positive number.\n";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    }
    
    int getOptionType() {
        int choice;
        while (true) {
            std::cout << "\nSelect Option Type:\n";
            std::cout << "1. Call Option\n";
            std::cout << "2. Put Option\n";
            std::cout << "Choice (1-2): ";
            
            if (std::cin >> choice && (choice == 1 || choice == 2)) {
                return choice;
            }
            std::cout << "Invalid choice. Please enter 1 or 2.\n";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    }
    
    int getExerciseType() {
        int choice;
        while (true) {
            std::cout << "\nSelect Exercise Type:\n";
            std::cout << "1. European\n";
            std::cout << "2. American\n";
            std::cout << "Choice (1-2): ";
            
            if (std::cin >> choice && (choice == 1 || choice == 2)) {
                return choice;
            }
            std::cout << "Invalid choice. Please enter 1 or 2.\n";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    }
    
    void displayResults(const Option& option) {
        std::cout << "\n" << std::string(60, '=') << "\n";
        std::cout << "OPTION PRICING RESULTS\n";
        std::cout << std::string(60, '=') << "\n";
        
        // Display option parameters
        std::cout << std::fixed << std::setprecision(2);
        std::cout << "Spot Price: $" << option.getSpot() << "\n";
        std::cout << "Strike Price: $" << option.getStrike() << "\n";
        std::cout << "Risk-free Rate: " << option.getRate() * 100 << "%\n";
        std::cout << "Volatility: " << option.getVolatility() * 100 << "%\n";
        std::cout << "Time to Maturity: " << option.getTimeToMaturity() << " years\n";
        std::cout << "Option Type: " << (option.getOptionType() == OptionType::CALL ? "Call" : "Put") << "\n";
        std::cout << "Exercise Type: " << (option.getExerciseType() == ExerciseType::EUROPEAN ? "European" : "American") << "\n\n";
        
        // Price using all available methods
        std::cout << "PRICING RESULTS:\n";
        std::cout << std::string(30, '-') << "\n";
        
        auto results = engine.priceAllMethods(option);
        std::cout << std::setprecision(4);
        for (const auto& result : results) {
            if (!std::isnan(result.second)) {
                std::cout << std::setw(15) << result.first << ": $" << result.second << "\n";
            } else {
                std::cout << std::setw(15) << result.first << ": Not Available\n";
            }
        }
        
        // Calculate and display Greeks (only for European options)
        if (option.getExerciseType() == ExerciseType::EUROPEAN) {
            std::cout << "\nGREEKS (Black-Scholes):\n";
            std::cout << std::string(30, '-') << "\n";
            
            auto greeks = engine.calculateGreeks(option);
            for (const auto& greek : greeks) {
                std::cout << std::setw(15) << greek.first << ": " << greek.second << "\n";
            }
        }
        
        std::cout << std::string(60, '=') << "\n";
    }
    
public:
    void run() {
        std::cout << "\n";
        std::cout << "╔══════════════════════════════════════════════════════════╗\n";
        std::cout << "║              OPTIONS PRICING ENGINE                      ║\n";
        std::cout << "║         Interactive Trading Tool Interface               ║\n";
        std::cout << "╚══════════════════════════════════════════════════════════╝\n\n";
        
        while (true) {
            try {
                // Get option parameters from user
                double spot = getValidInput("Enter spot price: $");
                double strike = getValidInput("Enter strike price: $");
                double rate = getValidInput("Enter risk-free rate (as decimal, e.g., 0.05 for 5%): ");
                double volatility = getValidInput("Enter volatility (as decimal, e.g., 0.2 for 20%): ");
                double timeToMaturity = getValidInput("Enter time to maturity (in years): ");
                
                int optionTypeChoice = getOptionType();
                OptionType optionType = (optionTypeChoice == 1) ? OptionType::CALL : OptionType::PUT;
                
                int exerciseTypeChoice = getExerciseType();
                ExerciseType exerciseType = (exerciseTypeChoice == 1) ? ExerciseType::EUROPEAN : ExerciseType::AMERICAN;
                
                // Create option and display results
                Option option(spot, strike, rate, volatility, timeToMaturity, optionType, exerciseType);
                displayResults(option);
                
                // Ask if user wants to continue
                char choice;
                std::cout << "\nPrice another option? (y/n): ";
                std::cin >> choice;
                
                if (choice != 'y' && choice != 'Y') {
                    std::cout << "\nThank you for using the Options Pricing Engine!\n";
                    break;
                }
                
            } catch (const std::exception& e) {
                std::cout << "Error: " << e.what() << "\n";
                std::cout << "Please try again.\n\n";
            }
        }
    }
};

int main() {
    InteractiveOptionsPricer pricer;
    pricer.run();
    return 0;
}
