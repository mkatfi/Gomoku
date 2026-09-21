#include "TranspositionTable.hpp"

TranspositionTable::TranspositionTable() {
    // A prime number for array size reduces hash collisions.
    table.resize(1000003);
}

void TranspositionTable::store(uint64_t key, int depth, int value, HashFlag flag, Point bestMove) {
    TTEntry& entry = table[key % table.size()];
    // Replace if we searched at least as deep (depth-preferred replacement).
    if (entry.zkey != key || entry.depth <= depth) {
        entry.zkey = key;
        entry.depth = depth;
        entry.value = value;
        entry.flag = flag;
        entry.moveR = bestMove.row;
        entry.moveC = bestMove.col;
    }
}

bool TranspositionTable::probe(uint64_t key, int depth, int alpha, int beta, int& outValue, Point& outMove) {
    const TTEntry& entry = table[key % table.size()];
    outMove = {-1, -1};

    if (entry.zkey != key) return false;

    outMove = {entry.moveR, entry.moveC}; // usable for ordering regardless of depth

    if (entry.depth >= depth) {
        if (entry.flag == HashFlag::EXACT) {
            outValue = entry.value;
            return true;
        }
        if (entry.flag == HashFlag::ALPHA && entry.value <= alpha) {
            outValue = alpha;
            return true;
        }
        if (entry.flag == HashFlag::BETA && entry.value >= beta) {
            outValue = beta;
            return true;
        }
    }
    return false;
}

void TranspositionTable::clear() {
    table.assign(table.size(), TTEntry{});
}
