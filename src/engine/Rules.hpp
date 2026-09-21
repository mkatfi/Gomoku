#ifndef RULES_HPP
#define RULES_HPP

#include "../core/Types.hpp"
#include "Board.hpp"
#include <vector>

class Rules {
public:
    // Core move validation
    static bool isLegalMove(const Board& board, int r, int c, Cell color);

    // Win conditions
    static bool hasFive(const Board& board, Cell color);
    static bool hasFiveAt(const Board& board, int r, int c, Cell color);
    static bool hasTenCaptures(const Board& board, Cell color);

    // Captures
    static std::vector<Point> checkCaptures(const Board& board, int r, int c, Cell color);

    // Advanced rules (Breakable 5)
    static bool isFiveBreakable(const Board& board, Cell winnerColor);
    
    // Advanced rules (Double Three)
    static bool isDoubleThree(const Board& board, int r, int c, Cell color);

private:
    // Helper functions for pattern detection
    static int countDirection(const Board& board, int r, int c, int dr, int dc, Cell color);
    static bool isOpenFourAt(const Board& board, int r, int c, int dr, int dc, Cell color);
    static bool isFreeThreeAt(const Board& board, int r, int c, int dr, int dc, Cell color);
    static bool checkIfCorrectFive(const Board& board, int r, int c, Cell opponentColor, Cell winnerColor);
    static void applyCaptures(Board& board, const std::vector<Point>& captured,
                              Cell color);
};

#endif // RULES_HPP