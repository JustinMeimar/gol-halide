#include <iostream>
#include "automata.h"

int main(int argc, char **argv) {

    std::string pulsar = "16b$"
        "16b$"
        "16b$"
        "16b$"
        "16b$"
        "16b$"
        "16b$"
        "7b1o8b$"
        "6b3o7b$"
        "7b1o8b$"
        "16b$"
        "16b$"
        "16b$"
        "16b$"
        "16b$"
        "16b$"
        "16b!";
    int x = 16;
    int y = 16;
    Automata gol(x, y, std::make_unique<ConwayRule>("b23/s2"),
                       std::make_unique<RunLengthEncoding>(std::move(pulsar))); 
    
    std::cout << "I am in main..\n";
    gol.simulate(1000);

    return 0;
}
