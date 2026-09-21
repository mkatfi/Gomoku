#include "Zobrist.hpp"

uint64_t Zobrist::keys[BOARD_SIZE][BOARD_SIZE][3];
uint64_t Zobrist::sideKey = 0;

void Zobrist::init() {
    // 64-bit random number generator
    std::mt19937_64 rng(12345); // Fixed seed so hashes are consistent

    for (int r = 0; r < BOARD_SIZE; ++r) {
        for (int c = 0; c < BOARD_SIZE; ++c) {
            keys[r][c][EMPTY] = 0; // Empty cells don't affect the hash
            keys[r][c][BLACK] = rng();
            keys[r][c][WHITE] = rng();
        }
    }
    sideKey = rng();
}

uint64_t Zobrist::getKey(int r, int c, Cell color) {
    return keys[r][c][color];
}

uint64_t Zobrist::getSideKey() {
    return sideKey;
}
