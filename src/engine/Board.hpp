#ifndef BOARD_HPP
#define BOARD_HPP

#include "../core/Types.hpp"
#include <iostream>
#include "Zobrist.hpp" 

class Board {
public:
    Board();

    // Accessors
    Cell getCell(int r, int c) const;
    void setCell(int r, int c, Cell cell);
    bool inBounds(int r, int c) const;

    // Captures
    int getCaptures(Cell color) const;
    void addCaptures(Cell color, int amount);
    void removeCaptures(Cell color, int amount);
    bool isFull() const;
    int emptyCount() const;

    // Debugging
    void print() const;
    
    // Zobrist Hashing
    uint64_t getHash() const;


private:
    Cell board[BOARD_SIZE][BOARD_SIZE];
    int capturedBlack;
    int capturedWhite;
    uint64_t currentHash;
};

#endif // BOARD_HPP