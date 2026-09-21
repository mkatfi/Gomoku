#ifndef ZOBRIST_HPP
#define ZOBRIST_HPP


#include "../core/Types.hpp"
#include <cstdint>
#include <random>

class Zobrist {

    public:
        
        static void init();

        // Get the random 64-bit number for a specific piece at a specific location
        static uint64_t getKey(int r, int c, Cell color);

        // Random key XORed in when it is the minimizing side's turn, so that
        // otherwise-identical boards with different side-to-move hash differently.
        static uint64_t getSideKey();
private:
    // [19][19][3] -> Row, Col, Color (0=Empty, 1=Black, 2=White)
    static uint64_t keys[BOARD_SIZE][BOARD_SIZE][3];
    static uint64_t sideKey;

};

#endif // ZOBRIST_HPP