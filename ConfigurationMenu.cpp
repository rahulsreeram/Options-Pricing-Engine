#include "ConfigurationMenu.h"

ConfigurationMenu::ConfigurationMenu(OptionsPricingEngine& engine) 
    : engine_(engine), binomialSteps_(1000), monteCarloSimulations_(100000) {}

int ConfigurationMenu::getValidIntInput(const std::string& prompt, int minValue) {
    int value;
    while (true) {
        std::cout << prompt;
        if (std::cin >> value && value >= minValue) {
            return value;
        }
        std::cout << "Invalid input. Please enter a number >= " << minValue << "\n";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
}

void ConfigurationMenu::displayCurrentConfig() {
    std::cout << "\n" << std::string(40, '=') << "\n";
    std::cout << "CURRENT CONFIGURATION\n";
    std::cout << std::string(40, '=') << "\n";
    std::cout << "Binomial Tree Steps: " << binomialSteps_ << "\n";
    std::cout << "Monte Carlo Simulations: " << monteCarloSimulations_ << "\n";
    std::cout << std::string(40, '=') << "\n";
}

void ConfigurationMenu::showMenu() {
    int choice;
    
    while (true) {
        displayCurrentConfig();
        std::cout << "\nCONFIGURATION MENU:\n";
        std::cout << "1. Configure Binomial Tree Steps\n";
        std::cout << "2. Configure Monte Carlo Simulations\n";
        std::cout << "3. Reset to Defaults\n";
        std::cout << "4. Return to Main Menu\n";
        std::cout << "Choice (1-4): ";
        
        if (std::cin >> choice) {
            switch (choice) {
                case 1:
                    configureBinomialSteps();
                    break;
                case 2:
                    configureMonteCarloSimulations();
                    break;
                case 3:
                    resetToDefaults();
                    break;
                case 4:
                    return;
                default:
                    std::cout << "Invalid choice. Please enter 1-4.\n";
            }
        } else {
            std::cout << "Invalid input. Please enter a number.\n";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    }
}

void ConfigurationMenu::configureBinomialSteps() {
    std::cout << "\nBinomial Tree Configuration:\n";
    std::cout << "Current steps: " << binomialSteps_ << "\n";
    std::cout << "Recommended range: 100-5000 (higher = more accurate but slower)\n";
    
    binomialSteps_ = getValidIntInput("Enter new number of steps: ", 10);
    engine_.setBinomialSteps(binomialSteps_);
    
    std::cout << "Binomial steps updated to: " << binomialSteps_ << "\n";
}

void ConfigurationMenu::configureMonteCarloSimulations() {
    std::cout << "\nMonte Carlo Configuration:\n";
    std::cout << "Current simulations: " << monteCarloSimulations_ << "\n";
    std::cout << "Recommended range: 10,000-1,000,000 (higher = more accurate but slower)\n";
    
    monteCarloSimulations_ = getValidIntInput("Enter new number of simulations: ", 1000);
    engine_.setMonteCarloSimulations(monteCarloSimulations_);
    
    std::cout << "Monte Carlo simulations updated to: " << monteCarloSimulations_ << "\n";
}

void ConfigurationMenu::resetToDefaults() {
    binomialSteps_ = 1000;
    monteCarloSimulations_ = 100000;
    
    engine_.setBinomialSteps(binomialSteps_);
    engine_.setMonteCarloSimulations(monteCarloSimulations_);
    
    std::cout << "Configuration reset to defaults.\n";
}
