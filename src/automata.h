#include <bitset>
#include <vector>

using RuleBitMap = std::vector<std::bitset<10>>;

///
/// 
template<size_t N, size_t K>
constexpr std::bitset<K> getLowestBits() {
    return std::bitset<K>(N & ((1ULL << K) - 1));
}

///
///
template<std::size_t N, std::size_t INDEX=0>
struct RulePermutationGenerator {
    /// 
    ///
    static void generate(RuleBitMap& rules) {
        // Check if we are generating past the specified permutations on N 
        if constexpr (INDEX < N) { 
            rules.push_back(getLowestBits<INDEX, 10>());
            RulePermutationGenerator<N, INDEX+1>::generate(rules);
        }
    }
};

class Rules {
public:
    /// Construct a set of rules for Conway's Game of Life.
    static Rules Conways() {
        RuleBitMap rules;
        RulePermutationGenerator<256>::generate(rules);
        
        return Rules(std::move(rules));
            
        // Replaces: 
        // return Rules(RuleBitMap{
            // std::bitset<10>("00000000"),  
            // std::bitset<10>("00000010"),  
            // std::bitset<10>("00000100"),  
            // std::bitset<10>("00000110"),  
            // std::bitset<10>("00001000"),  
        // });
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
    Automata(Rules rules) {}

    ///
private:
    /// update 
    void step();

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
