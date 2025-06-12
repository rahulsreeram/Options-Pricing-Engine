# Options Pricing Engine

A high-performance C++ options pricing engine implementing multiple valuation methodologies for European and American options. Designed with modularity and extensibility in mind, this engine provides accurate pricing under varying market conditions and supports comprehensive Greeks calculation.

## Features

### Pricing Methods
- **Black-Scholes Model**: Analytical closed-form solution for European options
- **Binomial Tree Model**: Discrete-time lattice approach supporting both European and American options
- **Monte Carlo Simulation**: Stochastic simulation with variance reduction techniques

### Option Types Supported
- European Call and Put Options
- American Call and Put Options (via Binomial Tree)
- Vanilla payoff structures with extensible framework for exotic options

### Greeks Calculation
- **Delta**: Price sensitivity to underlying asset price changes
- **Gamma**: Rate of change of delta
- **Theta**: Time decay (per day)
- **Vega**: Volatility sensitivity (per 1% volatility change)
- **Rho**: Interest rate sensitivity (per 1% rate change)

### Advanced Features
- Object-oriented design with clean separation of concerns
- Memory-efficient implementations optimized for performance
- Comprehensive error handling and input validation
- Cross-validation between pricing methods
- Extensible architecture for future enhancements

## Architecture

### Core Components

**Option Class**: Encapsulates option parameters including spot price, strike price, risk-free rate, volatility, time to maturity, and option type.

**PricingEngine Interface**: Abstract base class defining the pricing contract, enabling polymorphic behavior across different pricing methodologies.

**Specialized Engines**:
- `BlackScholesEngine`: Implements analytical Black-Scholes formulas
- `BinomialEngine`: Cox-Ross-Rubinstein binomial tree implementation
- `MonteCarloEngine`: Geometric Brownian Motion simulation with variance reduction

**OptionsPricingEngine**: Facade class providing unified interface to all pricing methods with configuration management.

## Requirements

- **Compiler**: C++14 compatible compiler (clang++, g++)
- **Platform**: Cross-platform (tested on macOS ARM64, Linux, Windows)
- **Dependencies**: Standard C++ library only

