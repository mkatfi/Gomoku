#ifndef TYPES_HPP
#define TYPES_HPP

#include <cstdint>
#include <vector>

const int BOARD_SIZE = 19;

enum Cell { EMPTY, BLACK, WHITE };

inline Cell getOpponent(Cell color) {
    if (color == BLACK) return WHITE;
    if (color == WHITE) return BLACK;
    return EMPTY;
}

struct Point {
    int row, col;
};

struct MoveResult {
    Point placed;
    Cell color;
    std::vector<Point> captured; // stones captured by this move
};

#endif // TYPES_HPP