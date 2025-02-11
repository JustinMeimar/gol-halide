#include <bitset>
#include <vector>
#include <array>
#include <memory>
#include <iostream>
#include <unordered_map>
#include <thread>
#include <atomic>
#include <optional>
#include <Halide.h>
#include "render.h"

using GridBuffer = Halide::Buffer<uint8_t>;

class Rule { 
    public:
        virtual ~Rule() = default; 
        virtual void apply(GridBuffer& read_buff, GridBuffer& write_buff,
                           const int w, const int h) = 0;
    
    protected:
        Halide::Func update_func;
        Halide::Var x, y;
        Halide::Target target;
};

class ConwayRule : public Rule {
    public:
        ConwayRule(std::string &&rule_string) {}

        virtual void apply(GridBuffer&, GridBuffer&, const int w, const int h) override;
};

/////////////////////////////////////////////////////////////////////
/// seed.h

class Seed {
  public:
    virtual GridBuffer allocate(std::size_t x, std::size_t y) = 0;
};

class RunLengthEncoding : public Seed {
  public:
    ///
    RunLengthEncoding(std::string&& seed_string)
        : seed_string(std::move(seed_string)) {}
    
    /// @desc: Unpack the grid run-length-encoding string into a buffer
    virtual GridBuffer allocate(std::size_t x, std::size_t y);

  private:
    std::string seed_string;
};


/////////////////////////////////////////////////////////////////////
/// automata.h


struct DualBuffer {

    DualBuffer(int w, int h)
        : w(w),
          h(h),
          buff_one(w, h),
          buff_two(w, h) {}
    
    int w, h;
    GridBuffer buff_one;
    GridBuffer buff_two;

    std::atomic<bool> active_buff{false}; // point to buff_one first
    
    const GridBuffer& getActiveBuffer() {
        return (active_buff) ? buff_two : buff_one;
    }

    /// Supply the lambda to apply to the buffer
    void apply_and_swap(std::unique_ptr<Rule>& r) {
        
        // use predicated excution to determine current buffer
        GridBuffer& read_buff = active_buff ? buff_two : buff_one;
        GridBuffer& write_buff = active_buff ? buff_one : buff_two; 
        
        // apply the computation to the other buffer
        r->apply(read_buff, write_buff, w, h);
        
        // other buffer becomes next read buffer
        active_buff = !active_buff; 
    }
};


struct AsyncFrameSequencer {
     
    AsyncFrameSequencer(int w, int h) : dual_buff(w, h) {}
    
    // members
    DualBuffer dual_buff;
    std::atomic<size_t> simulated_frames{0};
    std::atomic<size_t> cur_render_frame{0};
    
    // the thread(s) used for simulating the automata 
    void sim_thread(std::unique_ptr<Rule>& rule, size_t ticks) { 
        for (int i=0; i<ticks; i++) {
            dual_buff.apply_and_swap(rule);
            simulated_frames++;
        }
    }

    // the thread used for rendering
    void render_thread(render::GridConfig&& config) {
        
        std::size_t last_rendered_frame = 0; 
        render::OpenGLContext context(std::move(config));
        while (!context.shouldClose()) {
            context.renderFrame(config, dual_buff.getActiveBuffer());
            context.swapBuffersAndPollEvents();
        }
    } 
};

class Automata {
public:
    Automata(std::size_t x, std::size_t y, std::unique_ptr<Rule> rule,
                                           std::unique_ptr<Seed> seed)
        : x(x),
          y(y),
          rule(std::move(rule)),
          seed(std::move(seed)),
          async_seq(x, y) {}
 
    /// @TODO
    void simulate(std::size_t ticks); 

private:
    std::size_t x;
    std::size_t y; 
    std::unique_ptr<Rule> rule;
    std::unique_ptr<Seed> seed;
    AsyncFrameSequencer async_seq;

    /// four for the simulation, one for the queue, another for the renderer
    std::vector<std::thread> threads;
};    

