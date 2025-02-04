#include <bitset>
#include <vector>
#include <array>
#include <iostream>
#include <unordered_map>
/// @TODO: Fix this
#include "/home/justin/install/Halide/build/include/Halide.h"

constexpr std::size_t CONWAYS_RULE_COUNT = 512;

using RuleBitMap = std::array<std::size_t, CONWAYS_RULE_COUNT>; 

/// Get a compile time bitset of the K lowest bits from an unsigned integer N 
template<size_t N, size_t K>
constexpr std::bitset<K> getLowestBits() {
    return std::bitset<K>(N & ((1ULL << K) - 1));
}

///
template<std::size_t N, std::size_t INDEX=0>
struct RulePermutationGenerator {
    ///
    static void generate(RuleBitMap& rules) {
        // Check if we are generating past the specified permutations on N 
        if constexpr (INDEX < N) {
            auto constexpr bit_string = getLowestBits<INDEX, 9>();
            if (bit_string.count() < 2 || bit_string.count() > 4) {
                // make cell dead
                rules[INDEX] = 0;
            } else {
                // make cell alive
                rules[INDEX] = 1;
            }
            RulePermutationGenerator<N, INDEX+1>::generate(rules);
        }
    }
};

class Rules {
public:
    /// Construct a set of rules for Conway's Game of Life.
    static Rules Conways() {
        RuleBitMap rules;
        RulePermutationGenerator<CONWAYS_RULE_COUNT>::generate(rules); 
        return Rules(std::move(rules));
    }

    RuleBitMap getRules() const { return rules; }
private:
    /// private constructor to enforce going through 
    explicit Rules(RuleBitMap&& rules)
        : rules(std::move(rules)) {}

    /// 
    RuleBitMap rules;
};

///////
class Automata {
public:

    Automata(Rules&& rules)
        : rules(std::move(rules)) {}
    
    /// Initiailize with a halide buffer
    Automata(Rules&& rules, Halide::Buffer<uint8_t>& board)
        : rules(std::move(rules)) {}
    
    /// Initiailize with a regular old heap allocation
    Automata(Rules&& rules, uint8_t *board_ptr, std::size_t width, std::size_t height)
        : rules(std::move(rules)) {}
    
    /// @TODO
    void step();
    
    /// @TODO
    void draw();

private:
    Rules rules;

};

/*
Interface:

auto conways = new Automata(Rules::Conway);

// start time
while (!max_iter) {
    conways.step();
    conways.draw();
}
// stop time

*/
