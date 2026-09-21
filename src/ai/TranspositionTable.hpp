#ifndef TRANSPOSITIONTABLE_HPP
#define TRANSPOSITIONTABLE_HPP

#include "../core/Types.hpp"
#include <cstdint>
#include <vector>

enum class HashFlag { EXACT, ALPHA, BETA };

struct TTEntry {
    uint64_t zkey = 0;
    int depth = -1;
    int value = 0;
    HashFlag flag = HashFlag::EXACT;
    int moveR = -1; // best move found for this position (for move ordering)
    int moveC = -1;
};

class TranspositionTable {
public:
    TranspositionTable();

    void store(uint64_t key, int depth, int value, HashFlag flag, Point bestMove);

    // Returns true when a usable score cutoff is found (outValue is set).
    // Always writes the stored best move into outMove ({-1,-1} if none) so the
    // caller can order moves even when no cutoff is available.
    bool probe(uint64_t key, int depth, int alpha, int beta, int& outValue, Point& outMove);

    void clear();

private:
    std::vector<TTEntry> table;
};

#endif // TRANSPOSITIONTABLE_HPP
