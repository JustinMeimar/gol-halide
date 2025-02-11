#include <iostream>
#include "automata.h"

int main(int argc, char **argv) {
    std::string pulsar = "4b2o4b2o4b$"
                    "4b2o4b2o4b$"
                    "16b$"
                    "2b2o8b2o2b$"
                    "2bo10bo2b$" 
                    "2bo10bo2b$"
                    "2b2o8b2o2b$"
                    "16b$"
                    "2b2o8b2o2b$"
                    "2bo10bo2b$"
                    "2bo10bo2b$"
                    "2b2o8b2o2b$"
                    "16b$"
                    "4b2o4b2o4b$"
                    "4b2o4b2o4b$"
                    "16b!"; 
    int x = 16;
    int y = 16;
    Automata gol(x, y, std::make_unique<ConwayRule>("b23/s2"),
                       std::make_unique<RunLengthEncoding>(std::move(pulsar))); 
    
    std::cout << "I am in main..\n";
    gol.simulate(10000);

    return 0;
}
