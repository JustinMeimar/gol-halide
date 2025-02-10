#include "automata.h"
#include <chrono>
#include <thread>

static void print_buffer(Halide::Buffer<uint8_t>& buff) {
    const std::size_t w = buff.width();
    const std::size_t h = buff.height();
    for (int j=0; j<h; j++) {
        for (int i=0; i<w; i++) {
            std::cout << (int)buff(i, j) << " ";
        }
        printf("\n");
    }
}

static void init_input(Halide::Buffer<uint8_t>& input) {
    int height = input.height();
    int width = input.width();
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            input(x, y) = (x + y) % 2;
        }
    }
}

Halide::Buffer<uint8_t> RunLengthEncoding::allocate(std::size_t x, std::size_t y) {
    auto buff = Halide::Buffer<uint8_t>(128, 128);
    buff.allocate();

    return buff;
}

void ConwayRule::apply(Halide::Buffer<uint8_t>& buffer,
                       const int width,
                       const int height) {
 
    Halide::Var x("x"), y("y"); 
    Halide::Func padded_input = Halide::BoundaryConditions::constant_exterior(
        buffer, 
        Halide::cast<uint8_t>(0),// out-of-bounds value
        0, width, // x bounds
        0, height // y bounds
    );
    
    Halide::RDom r(-1, 3, -1, 3);  // 2D iteration from -1 to 1
    Halide::Func neighborSum;
    neighborSum(x, y) = Halide::sum(padded_input(x + r.x, y + r.y)) - padded_input(x, y);
    
    // CGOL rules
    Halide::Func nextState;
    nextState(x, y) = Halide::select(
        padded_input(x, y) == 1,
        // live cells
        Halide::select(
            neighborSum(x, y) == 2 || neighborSum(x, y) == 3,
            Halide::cast<uint8_t>(1),  // survival
            Halide::cast<uint8_t>(0)   // death
        ),
        // dead cells
        Halide::select(
            neighborSum(x, y) == 3,
            Halide::cast<uint8_t>(1),  // birth
            Halide::cast<uint8_t>(0)   // stay dead
        )
    );
     
    // temporary buffer for the result
    Halide::Buffer<uint8_t> temp(width, height);
    nextState.realize(temp);
    
    // copy result back to input buffer
    buffer.copy_from(temp);
}

/////////////////////////////////////////////////////////////////////
void Automata::simulate(std::size_t ticks) {
    printf("Here, I am\n"); 
    
    // printf("Computed the pipeline\n");
    const int width = 1000;
    const int height = 1000;
    
    // allocate a rendering config
    render::GridConfig config = {
        .size = 80,
        .cellSize = 2.0f / 80,
        .windowWidth = 800,
        .windowHeight = 800,
        .title = "Sim!",
        .maxTicks = ticks,
    }; 

    std::thread t(&render::GridApplication::run, std::move(config));

    Halide::Buffer<uint8_t> input(width, height);
    init_input(input);
    
    auto start = std::chrono::high_resolution_clock::now();

    for (int i=0; i<100; i++) {
        rule->apply(input, width, height);
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration_ms = end - start;
    std::chrono::duration<double> duration_sec = end - start;

    // Output the duration
    std::cout << "Duration: " << duration_ms.count() << " ms (" 
              << duration_sec.count() << " s)" << std::endl;

    t.join();
}


