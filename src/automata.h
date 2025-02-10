#include <bitset>
#include <vector>
#include <array>
#include <memory>
#include <iostream>
#include <unordered_map>

#include <Halide.h>
#include "render.h"

#define BLACK 0x00
#define WHITE 0xFF

using GridBuffer = Halide::Buffer<uint8_t>;

class Rule { 
    public:
        virtual ~Rule() = default; 
        virtual void apply(GridBuffer& buff, const int w, const int h) = 0;
    
    protected:
        Halide::Func update_func;
        Halide::Var x, y;
        Halide::Target target;
};

class ConwayRule : public Rule {
    public:
        ConwayRule(std::string &&rule_string) {}

        virtual void apply(GridBuffer& buff, const int w, const int h) override;
        // 
        virtual void define_and_sched(); 
};

class HexagonalRule : public Rule {
  public:
    virtual void apply(GridBuffer& buff, const int w, const int h) override;
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

class Automata {
public:
    Automata(std::size_t x, std::size_t y, Rule* rule, Seed* seed)
        : x(x),
          y(y),
          rule(rule),
          seed(seed) {}
 
    /// @TODO
    void simulate(std::size_t ticks); 

private:
    std::size_t x;
    std::size_t y; 
    Rule* rule;
    Seed* seed;
    
    /// @TODO: Figure how to properly allocate and atomic swap these.
    /// Internal buffers
    GridBuffer sim_buff;
    GridBuffer draw_buff; 
};    

