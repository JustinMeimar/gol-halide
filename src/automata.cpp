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
    auto buff = Halide::Buffer<uint8_t>(x, y);
    buff.allocate();
    return buff;
}

void ConwayRule::apply(Halide::Buffer<uint8_t>& read_buff,
                       Halide::Buffer<uint8_t>& write_buff,               
                       const int width,
                       const int height) {
 
    Halide::Var x("x"), y("y"); 
    Halide::Func padded_input = Halide::BoundaryConditions::constant_exterior(
        read_buff, 
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
    // Halide::Buffer<uint8_t> temp(width, height);
    nextState.realize(write_buff);
    
    // copy result back to input buffer
    read_buff.copy_from(write_buff);
}

/////////////////////////////////////////////////////////////////////
void Automata::simulate(std::size_t ticks) {
    printf("Here, I am\n"); 
    
    GridBuffer initial = seed->allocate(x, y);
    async_seq.dual_buff.buff_one.copy_from(initial);


    render::GridConfig config{
        .size = static_cast<int>(x),
        .cellSize = 4.0f / static_cast<float>(x),
        .windowWidth = 800,
        .windowHeight = 800,
        .title = "Automata Simulation",
        .maxTicks = ticks
    };

    // // Launch simulation threads (let's use 4 threads)
    // constexpr size_t NUM_SIM_THREADS = 4;
    // size_t ticks_per_thread = ticks / NUM_SIM_THREADS;
    
    // Create and launch simulation thread
    threads.emplace_back(&AsyncFrameSequencer::sim_thread,
                       &async_seq,
                       std::ref(rule),
                       ticks);

    // Launch render thread
    threads.emplace_back(&AsyncFrameSequencer::render_thread,
                        &async_seq,
                        std::move(config));

    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }
}

