#include <iostream>
#include "automata.h"

int main(int argc, char **argv) {
        
    int x = 16;
    int y = 16;
    Automata gol(x, y, std::make_unique<ConwayRule>("b23/s2"),
                       std::make_unique<RunLengthEncoding>("8b8o$8o8b!")); 
    
    std::cout << "I am in main..\n";
    gol.simulate(10000);

    return 0;
}
