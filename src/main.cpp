#include <iostream>
#include "automata.h"

int main(int argc, char **argv) {
        
    int x = 16;
    int y = 2;
    Automata gol(x, y, new ConwayRule("b23/s2"), new RunLengthEncoding("8b8o$8o8b!")); 
    
    std::cout << "I am in main..\n";
    gol.simulate(10);

    return 0;
}
