#include <iostream>
#include "automata.h"

int main() {

    auto rules = Rules::Conways();
    for (const auto& r : rules.getRules()) {
        std::cout << "RULE: " << r << std::endl; 
    }
    // std::cout << getLowestBits<255,6>() << std::endl; 
    // std::cout << "Hi\n";
    return 0;
}
