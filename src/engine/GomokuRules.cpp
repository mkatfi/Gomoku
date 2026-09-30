#include "GomokuRules.hpp"

// -------------------------
// Core Mechanics & Helpers
// -------------------------

int GomokuRules::countDirection(const Board& board, int r, int c, int dr, int dc, Cell color) {
    int count = 0;
    int nr = r + dr;
    int nc = c + dc;
    while (board.inBounds(nr, nc) && board.getCell(nr, nc) == color) {
        count++;
        nr += dr;
        nc += dc;
    }
    return count;
}

std::vector<Point> GomokuRules::findCapturedStones(const Board& board, int r, int c, Cell color) {
    std::vector<Point> captured;
    if (!board.inBounds(r, c) || (color != BLACK && color != WHITE))
        return captured;

    Cell opp = getOpponent(color);

    const int dr[] = { -1, -1, -1,  0, 0,  1, 1, 1 };
    const int dc[] = { -1,  0,  1, -1, 1, -1, 0, 1 };

    for (int d = 0; d < 8; d++) {
        int r1 = r + dr[d],     c1 = c + dc[d];
        int r2 = r + 2*dr[d],   c2 = c + 2*dc[d];
        int r3 = r + 3*dr[d],   c3 = c + 3*dc[d];

        if (board.inBounds(r1, c1) && board.inBounds(r2, c2) && board.inBounds(r3, c3)) {
            if (board.getCell(r1, c1) == opp &&
                board.getCell(r2, c2) == opp &&
                board.getCell(r3, c3) == color) 
            {
                captured.push_back({r1, c1});
                captured.push_back({r2, c2});
            }
        }
    }
    return captured;
}

void GomokuRules::applyCaptures(Board& board, const std::vector<Point>& captured, Cell color) {
    for (const Point& p : captured)
        board.setCell(p.row, p.col, EMPTY);
    board.addCaptures(color, static_cast<int>(captured.size()));
}

// -------------------------
// Win Conditions
// -------------------------

bool GomokuRules::hasFiveInRowAt(const Board& board, int r, int c, Cell color) {
    int dr[] = {0, 1, 1, 1};
    int dc[] = {1, 0, 1, -1};
    
    for (int i = 0; i < 4; i++) {
        int total = 1 + countDirection(board, r, c, dr[i], dc[i], color) 
                      + countDirection(board, r, c, -dr[i], -dc[i], color);
        if (total >= 5) return true;
    }
    return false;
}

bool GomokuRules::hasFiveInRow(const Board& board, Cell color) {
    for (int r = 0; r < BOARD_SIZE; r++) {
        for (int c = 0; c < BOARD_SIZE; c++) {
            if (board.getCell(r, c) == color && hasFiveInRowAt(board, r, c, color))
                return true;
        }
    }
    return false;
}

bool GomokuRules::hasCaptureWin(const Board& board, Cell color) {
    return board.getCaptures(color) >= 10;
}

// -------------------------
// Advanced Rules (Breakable Five)
// -------------------------

bool GomokuRules::checkIfCorrectFive(const Board& board, int r, int c, Cell opponentColor, Cell winnerColor) {
    // The refutation must itself be a legal move. In particular, the opponent
    // cannot "break" a five with a forbidden double-three move.
    if (!isLegalMove(board, r, c, opponentColor))
        return false;

    Board simBoard = board;
    simBoard.setCell(r, c, opponentColor);

    std::vector<Point> caps = findCapturedStones(simBoard, r, c, opponentColor);
    if (caps.empty()) return false; // a move that captures nothing can't refute the five

    applyCaptures(simBoard, caps, opponentColor);

    // The five is refuted (game continues / opponent wins) if this capturing move
    // either:
    //   (a) breaks the alignment so the winner no longer has five, OR
    //   (b) brings the opponent to ten captures (five pairs) — per the subject's
    //       endgame-capture rule, a capture win takes priority even if the line
    //       itself stays intact.
    if (!hasFiveInRow(simBoard, winnerColor)) return true;
    if (board.getCaptures(opponentColor) + (int)caps.size() >= 10) return true;

    return false;
}

bool GomokuRules::isFiveBreakable(const Board& board, Cell winnerColor) {
    Cell opponentColor = getOpponent(winnerColor);
    for (int r = 0; r < BOARD_SIZE; r++) {
        for (int c = 0; c < BOARD_SIZE; c++) {
            if (board.getCell(r, c) != EMPTY) continue;
            
            if (checkIfCorrectFive(board, r, c, opponentColor, winnerColor))
                return true;
        }
    }
    return false;
}

// -------------------------
// Advanced Rules (Double Three)
// -------------------------

bool GomokuRules::isOpenFourAt(const Board& board, int r, int c, int dr, int dc, Cell color) {
    int startR = r, startC = c;
    
    while (board.inBounds(startR - dr, startC - dc) && board.getCell(startR - dr, startC - dc) == color) {
        startR -= dr;
        startC -= dc;
    }
    
    int count = 1 + countDirection(board, startR, startC, dr, dc, color);
    if (count != 4) return false;

    int beforeR = startR - dr;
    int beforeC = startC - dc;
    bool openBefore = board.inBounds(beforeR, beforeC) && board.getCell(beforeR, beforeC) == EMPTY;
    
    int afterR = startR + 4 * dr;
    int afterC = startC + 4 * dc;
    bool openAfter = board.inBounds(afterR, afterC) && board.getCell(afterR, afterC) == EMPTY;
    
    return openBefore && openAfter;
}

bool GomokuRules::isFreeThreeAt(const Board& board, int r, int c, int dr, int dc, Cell color) {
    for (int offset = -3; offset <= 3; offset++) {
        int tr = r + (offset * dr);
        int tc = c + (offset * dc);

        if (board.inBounds(tr, tc) && board.getCell(tr, tc) == EMPTY) {
            Board simBoard = board;
            simBoard.setCell(tr, tc, color);
            
            if (isOpenFourAt(simBoard, tr, tc, dr, dc, color)) {
                // The four must be created by the move under inspection. This
                // prevents an unrelated open-four elsewhere on the same axis
                // from being counted as a free-three.
                int back = countDirection(simBoard, tr, tc, -dr, -dc, color);
                int forward = countDirection(simBoard, tr, tc, dr, dc, color);
                int startR = tr - back * dr;
                int startC = tc - back * dc;
                bool originalMoveIsInRun = false;
                for (int i = 0; i <= back + forward; ++i) {
                    if (startR + i * dr == r && startC + i * dc == c) {
                        originalMoveIsInRun = true;
                        break;
                    }
                }
                if (originalMoveIsInRun)
                    return true;
            }
        }
    }
    return false;
}

bool GomokuRules::isDoubleThree(const Board& board, int r, int c, Cell color) {
    if (!board.inBounds(r, c) || board.getCell(r, c) != EMPTY)
        return false;

    Board simBoard = board;
    simBoard.setCell(r, c, color); // Simulate placing the piece

    int count = 0;
    int dr[] = {0, 1, 1, 1};
    int dc[] = {1, 0, 1, -1};
    
    for (int i = 0; i < 4; i++)
        if (isFreeThreeAt(simBoard, r, c, dr[i], dc[i], color)) count++;
    return count > 1;
}

// -------------------------
// General Move Validation
// -------------------------

bool GomokuRules::isLegalMove(const Board& board, int r, int c, Cell color) {
    if (color != BLACK && color != WHITE)
        return false;
    if (!board.inBounds(r, c) || board.getCell(r, c) != EMPTY) {
        return false;
    }

    // Double-Three rule exception: legal if it results in a capture
    if (isDoubleThree(board, r, c, color)) {
        if (findCapturedStones(board, r, c, color).empty()) {
            return false;
        }
    }
    
    return true;
}