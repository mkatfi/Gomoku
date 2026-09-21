#ifndef AI_HPP
#define AI_HPP

#include "../engine/GameEngine.hpp"
#include "../core/Types.hpp"
#include "TranspositionTable.hpp"
#include <chrono>
#include <exception>

class AI {
public:
    // Takes the engine state and returns the best Point to play.
    Point getBestMove(GameEngine& engine, Cell aiColor);

    // ===== BONUS (AI difficulty selector) =====
    // Set the per-move thinking budget in milliseconds. A bigger budget lets the
    // search reach deeper, so Easy/Medium/Hard simply map to different values.
    void setTimeLimit(int ms) { maxTimeMs = ms; }

private:
    static const int MAX_PLY = 64;

    // Information needed to undo a single search move (stone + any captures).
    struct Undo {
        int r, c;
        Cell color;
        // A single move can capture a pair in each of the eight directions.
        // Keep room for all sixteen removed stones; the old eight-element
        // buffer could overflow on a legal multi-direction capture.
        Point caps[16];
        int nCaps;
    };

    TranspositionTable tt;

    // --- Time management ---
    std::chrono::time_point<std::chrono::steady_clock> startTime;
    long long nodeCount = 0;
    int maxTimeMs = 420; // BONUS (AI difficulty): per-move budget, set via setTimeLimit()
    long long getElapsedTime() const;
    bool timeIsUp();
    class TimeOutException : public std::exception {};

    // --- Move-ordering memory ---
    Point killers[MAX_PLY][2];
    int historyTable[BOARD_SIZE][BOARD_SIZE];

    // Reusable per-ply move buffers so the search allocates no heap memory in
    // its hot loop.
    std::vector<Point> moveBuf[MAX_PLY];

    // --- Fast incremental board mutation (no rule/legality/full-scan overhead) ---
    // Returns true if this move is an immediate win for `color`.
    bool applySearchMove(Board& b, int r, int c, Cell color, Undo& u);
    void undoSearchMove(Board& b, const Undo& u);

    // Negamax with alpha-beta. Score is from `color`'s point of view.
    int negamax(Board& b, int depth, int ply, int alpha, int beta, Cell color,
                bool previousFivePending);
    int terminalScore(const Board& b, Cell color, bool previousFivePending) const;

    // Orders candidate moves in place (best first) using TT move, killers,
    // history and the static tactical score, and truncates to `cap`.
    void orderMoves(const Board& b, std::vector<Point>& moves, Cell color,
                    int ply, Point ttMove, int cap);
};

#endif // AI_HPP
