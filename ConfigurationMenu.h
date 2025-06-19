#ifndef CONFIGURATION_MENU_H
#define CONFIGURATION_MENU_H

#include "OptionsPricingEngine.h"
#include <iostream>
#include <limits>

class ConfigurationMenu {
private:
    OptionsPricingEngine& engine_;
    int binomialSteps_;
    int monteCarloSimulations_;
    
    int getValidIntInput(const std::string& prompt, int minValue = 1);
    void displayCurrentConfig();
    
public:
    ConfigurationMenu(OptionsPricingEngine& engine);
    void showMenu();
    void configureBinomialSteps();
    void configureMonteCarloSimulations();
    void resetToDefaults();
};

#endif
