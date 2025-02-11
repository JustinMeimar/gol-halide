#include <iostream>
#include "automata.h"
#include <filesystem>

namespace fs = std::filesystem;

int main(int argc, char **argv) {
    
    if (argc < 2) {
        printf("usage: <LD_PRELOAD> ./gol <seed_path>");
        return 1;
    }
    
    const char* seed_path_arg = argv[1];
    fs::path seed_path(seed_path_arg);
    if (!fs::exists(seed_path) || seed_path.extension() != ".rle") {
        std::cerr << "Failed to open path: " << seed_path_arg << std::endl;
        return 1;
    }
    
    auto seed = Seed2D::parse(seed_path);
    
    Automata gol(seed.x, seed.y, 
                 std::make_unique<ConwayRule>("b23/s2"),
                 std::make_unique<Seed2D>(std::move(seed)));
    
    std::cout << "I am in main..\n";
    gol.simulate(1000); 

    return 0;
}
