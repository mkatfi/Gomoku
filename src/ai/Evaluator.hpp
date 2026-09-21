#ifndef EVALUATOR_HPP
#define EVALUATOR_HPP

#include "../engine/Board.hpp"
#include "../core/Types.hpp"
#include "algorithm"

class Evaluator {
public:
    // Evaluates the board from the perspective of the AI color.
    // Positive score means AI is winning, negative means opponent is winning.
    static int evaluate(const Board& board, Cell aiColor);

private:
    // Scores a specific line on the board
    static int evaluateLine(int count, int openEnds, bool isAI);
    
    // Scans a direction to count stones and open ends
    static void countConsecutive(const Board& board, int r, int c, int dr, int dc, Cell color, int& count, int& openEnds);
};

#endif 