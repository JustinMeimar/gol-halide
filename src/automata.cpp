#include "automata.h"

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

Seed2D Seed2D::parse(fs::path& seed_path) {
    int x = 0, y = 0;
    std::string seed_content;
    std::ifstream seed_file(seed_path);
    std::string line;
    
    while (std::getline(seed_file, line)) {
        
        if (line.empty()) continue; // empty line
        if (line[0] == '#') continue; // comments
        
        // Check for header line containing dimensions and rules
        size_t x_pos = line.find("x = ");
        if (x_pos != std::string::npos) {
            x_pos += 4;  // skip 'x = '
            size_t x_end = line.find(",", x_pos);
            if (x_end != std::string::npos) {
                x = std::stoi(line.substr(x_pos, x_end - x_pos));
            } 
            size_t y_pos = line.find("y = ");
            if (y_pos != std::string::npos) {
                y_pos += 4; // skip 'y = ' 
                size_t y_end = line.find(",", y_pos);
                if (y_end != std::string::npos) {
                    y = std::stoi(line.substr(y_pos, y_end - y_pos));
                }
            }
            /// @TODO: Parse the rule field
            continue;
        }
        
        size_t percent_pos = line.find('%');
        if (percent_pos != std::string::npos) {
            line = line.substr(0, percent_pos);
        }
        
        size_t exclaim_pos = line.find('!');
        if (exclaim_pos != std::string::npos) {
            line = line.substr(0, exclaim_pos);
            seed_content += line;
            break; 
        } 
        seed_content += line;
    } 
    if (x == 0 || y == 0) {
        throw std::runtime_error("Invalid RLE file format: missing or invalid dimensions");
    }
    
    return Seed2D(x, y, std::move(seed_content));
}

Halide::Buffer<uint8_t> Seed2D::allocate(std::size_t x, std::size_t y) {
    auto buff = Halide::Buffer<uint8_t>(x, y);
    buff.fill(0);

    int cur_x = 0;
    int cur_y = 0;
    for (auto it = seed_content.begin(); it != seed_content.end(); it++) {
        if (*it == '!') break;
        
        int run_length = 0;
        while (it != seed_content.end() && std::isdigit(*it)) {
            run_length *= 10;
            run_length += static_cast<int>(*it - '0');
            it++;
        }
        if (run_length == 0) run_length = 1;
        
        if (*it == '$') {
            cur_y++;
            cur_x = 0;
            continue;
        }
        
        if (cur_x < x && cur_y < y) {
            for (int i = 0; i < run_length && cur_x < x; i++) {
                buff(cur_x++, cur_y) = (*it == 'o') ? 1 : 0;
            }
        }
    } 
    return buff;
}

void ConwayRule::apply(Halide::Buffer<uint8_t>& read_buff,
                       Halide::Buffer<uint8_t>& write_buff,               
                       const int width,
                       const int height) {
    Halide::Var x("x"), y("y"); 
    
    // Create edge-padded input
    Halide::Func padded_input = Halide::BoundaryConditions::constant_exterior(
        read_buff, 
        Halide::cast<uint8_t>(0),
        0, width,
        0, height
    );
    
    // Count neighbors (excluding center cell)
    Halide::RDom r(-1, 3, -1, 3);
    Halide::Func neighborSum;
    neighborSum(x, y) = Halide::sum(padded_input(x + r.x, y + r.y)) - padded_input(x, y);
    
    // Conway's rules
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

    // Direct realization into output buffer
    // nextState.realize({width, height});
    Halide::Realization realization({write_buff});
    nextState.realize(realization);
}

/////////////////////////////////////////////////////////////////////
void Automata::simulate(std::size_t ticks) {
    
    GridBuffer initial = seed->allocate(x, y);
    async_seq.dual_buff.buff_one.copy_from(initial);
    async_seq.dual_buff.buff_two.copy_from(initial);
    
    // Create the grid config
    render::GridConfig config(x, y, "Sim", ticks); 

    // Create and launch simulation thread
    threads.emplace_back(&AsyncFrameSequencer::sim_thread,
                       &async_seq,
                       std::ref(rule),
                       ticks);

#ifdef RENDER
    // Launch render thread
    threads.emplace_back(&AsyncFrameSequencer::render_thread,
                        &async_seq,
                        std::move(config));
    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }
#else
    threads.at(0).join();
#endif
 
}

