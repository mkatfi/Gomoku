#include "MoveGenerator.hpp"

// 4 line axes (horizontal, vertical, two diagonals) and 8 surrounding offsets.
static const int AXIS_DR[4] = {0, 1, 1, 1};
static const int AXIS_DC[4] = {1, 0, 1, -1};
static const int DIR8_DR[8] = {-1, -1, -1,  0, 0,  1, 1, 1};
static const int DIR8_DC[8] = {-1,  0,  1, -1, 1, -1, 0, 1};

int MoveGenerator::countDir(const Board& board, int r, int c, int dr, int dc, Cell who) {
    int n = 0;
    int nr = r + dr, nc = c + dc;
    while (board.inBounds(nr, nc) && board.getCell(nr, nc) == who) {
        n++;
        nr += dr;
        nc += dc;
    }
    return n;
}

int MoveGenerator::lineWeight(int runLength, int openEnds) {
    // runLength = friendly stones that would join the placed stone on this axis.
    int total = runLength + 1; // include the stone we are placing
    if (total >= 5) return 1000000;            // makes/blocks a five
    if (total == 4) return openEnds == 2 ? 100000 : 20000;
    if (total == 3) return openEnds == 2 ? 8000  : 1500;
    if (total == 2) return openEnds == 2 ? 600   : 120;
    return 10;
}

int MoveGenerator::staticScore(const Board& board, int r, int c, Cell color) {
    Cell opp = getOpponent(color);
    int score = 0;

    // 1. Capture potential: placing here flanks an opponent pair (color, opp, opp, <here>).
    for (int d = 0; d < 8; ++d) {
        int r1 = r + DIR8_DR[d],     c1 = c + DIR8_DC[d];
        int r2 = r + 2 * DIR8_DR[d], c2 = c + 2 * DIR8_DC[d];
        int r3 = r + 3 * DIR8_DR[d], c3 = c + 3 * DIR8_DC[d];
        if (board.inBounds(r3, c3) &&
            board.getCell(r1, c1) == opp &&
            board.getCell(r2, c2) == opp &&
            board.getCell(r3, c3) == color) {
            score += 12000;
        }
    }

    // 2. Offense (extend our lines) and defense (block opponent lines) per axis.
    for (int a = 0; a < 4; ++a) {
        int dr = AXIS_DR[a], dc = AXIS_DC[a];

        // --- Our own line through this point ---
        int ownF = countDir(board, r, c, dr, dc, color);
        int ownB = countDir(board, r, c, -dr, -dc, color);
        int ownEnds = 0;
        if (board.inBounds(r + (ownF + 1) * dr, c + (ownF + 1) * dc) &&
            board.getCell(r + (ownF + 1) * dr, c + (ownF + 1) * dc) == EMPTY) ownEnds++;
        if (board.inBounds(r - (ownB + 1) * dr, c - (ownB + 1) * dc) &&
            board.getCell(r - (ownB + 1) * dr, c - (ownB + 1) * dc) == EMPTY) ownEnds++;
        score += lineWeight(ownF + ownB, ownEnds) * 2; // attacking weighs more

        // --- Opponent's line we would interrupt by sitting here ---
        int oppF = countDir(board, r, c, dr, dc, opp);
        int oppB = countDir(board, r, c, -dr, -dc, opp);
        int oppEnds = 0;
        if (board.inBounds(r + (oppF + 1) * dr, c + (oppF + 1) * dc) &&
            board.getCell(r + (oppF + 1) * dr, c + (oppF + 1) * dc) == EMPTY) oppEnds++;
        if (board.inBounds(r - (oppB + 1) * dr, c - (oppB + 1) * dc) &&
            board.getCell(r - (oppB + 1) * dr, c - (oppB + 1) * dc) == EMPTY) oppEnds++;
        score += lineWeight(oppF + oppB, oppEnds);
    }

    return score;
}

void MoveGenerator::candidates(const Board& board, int radius, std::vector<Point>& out) {
    // Use a monotonically increasing stamp instead of clearing a 361-cell array
    // on every call (this runs at every search node, so the clear cost matters).
    static thread_local int seen[BOARD_SIZE][BOARD_SIZE] = {};
    static thread_local int stamp = 0;
    ++stamp;

    out.clear();
    bool anyStone = false;

    for (int r = 0; r < BOARD_SIZE; ++r) {
        for (int c = 0; c < BOARD_SIZE; ++c) {
            if (board.getCell(r, c) == EMPTY) continue;
            anyStone = true;
            // Mark every empty intersection within `radius` of this stone.
            for (int dr = -radius; dr <= radius; ++dr) {
                for (int dc = -radius; dc <= radius; ++dc) {
                    int nr = r + dr, nc = c + dc;
                    if (!board.inBounds(nr, nc)) continue;
                    if (seen[nr][nc] == stamp) continue;
                    if (board.getCell(nr, nc) != EMPTY) continue;
                    seen[nr][nc] = stamp;
                    out.push_back({nr, nc});
                }
            }
        }
    }

    if (!anyStone) out.push_back({BOARD_SIZE / 2, BOARD_SIZE / 2});
}

std::vector<Point> MoveGenerator::candidates(const Board& board, int radius) {
    std::vector<Point> result;
    candidates(board, radius, result);
    return result;
}
