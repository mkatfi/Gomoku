#include "Evaluator.hpp"
#include "../engine/Rules.hpp"



int Evaluator::evaluate(const Board& board, Cell aiColor) {
    int score = 0;
    Cell oppColor = getOpponent(aiColor);

    // 1. Evaluate Captures (Huge priority for the Pente variant)
    int aiCaptures = board.getCaptures(aiColor);
    int oppCaptures = board.getCaptures(oppColor);
    
    if (aiCaptures >= 10) return 1000000;  // Instant win
    if (oppCaptures >= 10) return -1000000; // Instant loss
    
    score += aiCaptures * 2000;
    score -= oppCaptures * 2000;

    // 2. Scan the board for lines (Horizontal, Vertical, 2 Diagonals)
    const int dr[] = {0, 1, 1, 1};
    const int dc[] = {1, 0, 1, -1};

    for (int r = 0; r < BOARD_SIZE; ++r) {
        for (int c = 0; c < BOARD_SIZE; ++c) {
            Cell cell = board.getCell(r, c);
            if (cell == EMPTY) continue;

            bool isAI = (cell == aiColor);

            for (int d = 0; d < 4; ++d) {
                int count = 0;
                int openEnds = 0;
                
                countConsecutive(board, r, c, dr[d], dc[d], cell, count, openEnds);
                
                // Only score lines from their starting point to avoid double-counting
                int prevR = r - dr[d];
                int prevC = c - dc[d];
                if (board.inBounds(prevR, prevC) && board.getCell(prevR, prevC) == cell) {
                    continue; 
                }

                score += evaluateLine(count, openEnds, isAI);
            }
        }
    }

    return score;
}

void Evaluator::countConsecutive(const Board& board, int r, int c, int dr, int dc, Cell color, int& count, int& openEnds) {
    count = 0;
    openEnds = 0;

    // Check backwards for an open end
    int prevR = r - dr;
    int prevC = c - dc;
    if (board.inBounds(prevR, prevC) && board.getCell(prevR, prevC) == EMPTY) {
        openEnds++;
    }

    // Count forward
    int currR = r;
    int currC = c;
    while (board.inBounds(currR, currC) && board.getCell(currR, currC) == color) {
        count++;
        currR += dr;
        currC += dc;
    }

    // Check forwards for an open end
    if (board.inBounds(currR, currC) && board.getCell(currR, currC) == EMPTY) {
        openEnds++;
    }
}

int Evaluator::evaluateLine(int count, int openEnds, bool isAI) {
    if (count >= 5) return isAI ? 100000 : -100000;
    
    int score = 0;
    if (count == 4) {
        if (openEnds == 2) score = 10000; // Open 4 (Unstoppable win)
        else if (openEnds == 1) score = 1000;  // Blocked 4
    } 
    else if (count == 3) {
        if (openEnds == 2) score = 1000;  // Open 3
        else if (openEnds == 1) score = 100;   // Blocked 3
    }
    else if (count == 2) {
        if (openEnds == 2) score = 100;   // Open 2
        else if (openEnds == 1) score = 10;    // Blocked 2
    }

    // If it's the opponent's line, we want to subtract the score
    // We multiply by 1.2 to make the AI slightly defensive (prefers blocking)
    return isAI ? score : -(score * 1.2); 
}