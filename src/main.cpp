#include <iostream>
#include "automata.h"

int main() {

    // auto rules = Rules::Conways();
    // for (const auto predicate : rules.getRules()) {
    //     std::cout << "RULE: " << predicate << std::endl; 
    // }
    
    Automata gol = Automata(Rules::Conways());
    while (1) {
        gol.step();
        gol.draw();
    }

    return 0;
}
