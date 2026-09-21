#ifndef MOVEGENERATOR_HPP
#define MOVEGENERATOR_HPP

#include "../engine/Board.hpp"
#include "../core/Types.hpp"
#include <vector>

// Reduces the 361 board intersections to a small set of relevant candidate
// moves and scores them cheaply (no board mutation, no full-board scans) so the
// alpha-beta search can try the most promising moves first.
class MoveGenerator {
public:
    // Empty cells within `radius` (Chebyshev distance) of any existing stone.
    // These are the only moves worth considering in Gomoku.
    static std::vector<Point> candidates(const Board& board, int radius = 2);

    // Same, but fills a caller-owned buffer to avoid per-node heap allocation.
    static void candidates(const Board& board, int radius, std::vector<Point>& out);

    // Cheap tactical score used purely for move ordering. Higher = try sooner.
    // Rewards creating captures, extending our own lines, and blocking the
    // opponent's lines. Does NOT mutate the board.
    static int staticScore(const Board& board, int r, int c, Cell color);

private:
    // Count consecutive `who` stones starting just past (r,c) along (dr,dc).
    static int countDir(const Board& board, int r, int c, int dr, int dc, Cell who);
    // Maps a run length around a placed stone to an ordering weight.
    static int lineWeight(int runLength, int openEnds);
};

#endif // MOVEGENERATOR_HPP
