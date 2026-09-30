#include "GomokuAI.hpp"
#include "MoveGenerator.hpp"
#include "Evaluator.hpp"
#include "../engine/GomokuRules.hpp"
#include <iostream>
#include <algorithm>
#include <limits>

namespace {
    const int INF        = 1000000000;
    const int WIN_SCORE  = 10000000;   // well above any heuristic evaluation

    // How many candidate moves to keep per node after ordering. A tight cap is
    // what lets alpha-beta reach deep plies inside the time budget.
    const int ROOT_CAP   = 16;
    const int INNER_CAP  = 10;

    const int DIR8_DR[8] = {-1, -1, -1,  0, 0,  1, 1, 1};
    const int DIR8_DC[8] = {-1,  0,  1, -1, 1, -1, 0, 1};
}

long long GomokuAI::getElapsedTime() const {
    auto now = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime).count();
}

bool GomokuAI::timeIsUp() {
    // Check often enough to leave headroom under the subject's 500 ms limit,
    // without paying for a clock read on every node.
    if ((++nodeCount & 127) != 0) return false;
    return getElapsedTime() >= maxTimeMs;
}

// --- Fast incremental make/undo (mirrors the engine's capture rule) ----------

void GomokuAI::applySearchMove(Board& b, int r, int c, Cell color, Undo& u) {
    Cell opp = getOpponent(color);
    u.r = r; u.c = c; u.color = color; u.nCaps = 0;

    b.setCell(r, c, color);

    // Capture any flanked opponent pairs around the placed stone.
    for (int d = 0; d < 8; ++d) {
        int r1 = r + DIR8_DR[d],     c1 = c + DIR8_DC[d];
        int r2 = r + 2 * DIR8_DR[d], c2 = c + 2 * DIR8_DC[d];
        int r3 = r + 3 * DIR8_DR[d], c3 = c + 3 * DIR8_DC[d];
        if (b.inBounds(r3, c3) &&
            b.getCell(r1, c1) == opp &&
            b.getCell(r2, c2) == opp &&
            b.getCell(r3, c3) == color) {
            b.setCell(r1, c1, EMPTY);
            b.setCell(r2, c2, EMPTY);
            u.caps[u.nCaps++] = {r1, c1};
            u.caps[u.nCaps++] = {r2, c2};
        }
    }
    if (u.nCaps > 0) b.addCaptures(color, u.nCaps);

    // Terminal status is evaluated by negamax using the exact engine rules.
    // Keeping this helper limited to make/undo makes it impossible for the AI
    // and GameEngine to disagree about breakable fives.
}

void GomokuAI::undoSearchMove(Board& b, const Undo& u) {
    if (u.nCaps > 0) {
        b.removeCaptures(u.color, u.nCaps);
        Cell opp = getOpponent(u.color);
        for (int i = 0; i < u.nCaps; ++i)
            b.setCell(u.caps[i].row, u.caps[i].col, opp);
    }
    b.setCell(u.r, u.c, EMPTY);
}

int GomokuAI::terminalScore(const Board& b, Cell color, bool previousFivePending) const {
    Cell opponent = getOpponent(color);

    if (GomokuRules::hasCaptureWin(b, color)) return WIN_SCORE;
    if (GomokuRules::hasCaptureWin(b, opponent)) return -WIN_SCORE;

    // A five is terminal only when it cannot be broken by a legal capture.
    if (GomokuRules::hasFiveInRow(b, color) &&
        !GomokuRules::isFiveBreakable(b, color))
        return WIN_SCORE;
    if (GomokuRules::hasFiveInRow(b, opponent)) {
        // Immediately after a breakable five is formed, the opponent gets one
        // move to capture it. Once that move has been made, a surviving line
        // is a win for its owner even if it remains technically breakable.
        if (!previousFivePending ||
            !GomokuRules::isFiveBreakable(b, opponent))
            return -WIN_SCORE;
    }
    return 0;
}

// --- Move ordering ------------------------------------------------------------

void GomokuAI::orderMoves(const Board& b, std::vector<Point>& moves, Cell color,
                    int ply, Point ttMove, int cap) {
    Point k0 = (ply < MAX_PLY) ? killers[ply][0] : Point{-1, -1};
    Point k1 = (ply < MAX_PLY) ? killers[ply][1] : Point{-1, -1};

    static thread_local std::vector<std::pair<int, Point>> scored;
    scored.clear();
    for (const Point& m : moves) {
        int s = MoveGenerator::scoreCandidateMove(b, m.row, m.col, color);
        if (m.row == ttMove.row && m.col == ttMove.col) s += 100000000; // PV move first
        else if ((m.row == k0.row && m.col == k0.col) ||
                 (m.row == k1.row && m.col == k1.col)) s += 5000000;    // killers
        s += historyTable[m.row][m.col];
        scored.push_back({s, m});
    }

    std::sort(scored.begin(), scored.end(),
              [](const auto& a, const auto& b) { return a.first > b.first; });

    int keep = std::min((int)scored.size(), cap);
    moves.resize(keep);
    for (int i = 0; i < keep; ++i) moves[i] = scored[i].second;
}

// --- Negamax search -----------------------------------------------------------

int GomokuAI::negamax(Board& b, int depth, int ply, int alpha, int beta, Cell color,
                bool previousFivePending) {
    if (timeIsUp()) throw TimeOutException();

    int terminal = terminalScore(b, color, previousFivePending);
    if (terminal != 0) return terminal;

    // Transposition table probe (key encodes stones + side-to-move + captures).
    uint64_t key = b.getHash();
    key ^= (uint64_t)b.getCaptures(BLACK) * 0x9E3779B97F4A7C15ULL;
    key ^= (uint64_t)b.getCaptures(WHITE) * 0xC2B2AE3D27D4EB4FULL;
    if (color == WHITE) key ^= Zobrist::getSideKey();

    int ttValue;
    Point ttMove{-1, -1};
    if (tt.probe(key, depth, alpha, beta, ttValue, ttMove)) {
        return ttValue;
    }

    if (depth <= 0) {
        return Evaluator::evaluatePosition(b, color);
    }

    // Wider neighbourhood near the root (don't miss tactical jumps); tighter
    // deeper down so the branching factor stays small and we reach depth.
    int radius = (ply <= 1) ? 2 : 1;
    std::vector<Point>& moves = moveBuf[ply < MAX_PLY ? ply : MAX_PLY - 1];
    MoveGenerator::generateCandidateMoves(b, radius, moves);
    moves.erase(std::remove_if(moves.begin(), moves.end(),
        [&](const Point& m) {
            return !GomokuRules::isLegalMove(b, m.row, m.col, color);
        }), moves.end());
    if (moves.empty()) return Evaluator::evaluatePosition(b, color);
    orderMoves(b, moves, color, ply, ttMove, INNER_CAP);

    int originalAlpha = alpha;
    int bestScore = -INF;
    Point bestMove = moves[0];
    Cell opp = getOpponent(color);
    int moveIndex = 0;

    for (const Point& m : moves) {
        Undo u;
        applySearchMove(b, m.row, m.col, color, u);

        int score;
        if (moveIndex == 0) {
            // Search the principal variation move at full depth and width.
            bool nextPending = GomokuRules::hasFiveInRow(b, color) &&
                               GomokuRules::isFiveBreakable(b, color);
            score = -negamax(b, depth - 1, ply + 1, -beta, -alpha, opp,
                             nextPending);
        } else {
            // Late Move Reductions: quiet moves ordered late are unlikely to be
            // best, so search them shallower first and only re-search at full
            // depth if they unexpectedly beat alpha. This is what lets the
            // nominal search depth reach 10+ inside the time budget.
            int reduction = 0;
            if (depth >= 3 && moveIndex >= 2 && u.nCaps == 0) {
                reduction = 1;
                if (moveIndex >= 4) reduction = 2;
                if (moveIndex >= 8) reduction = depth / 3 + 1;
                if (reduction > depth - 1) reduction = depth - 1; // keep depth >= 0
            }
            // Null-window probe (PVS), possibly reduced.
            bool nextPending = GomokuRules::hasFiveInRow(b, color) &&
                               GomokuRules::isFiveBreakable(b, color);
            score = -negamax(b, depth - 1 - reduction, ply + 1,
                             -alpha - 1, -alpha, opp, nextPending);
            // Re-search at full depth/width if it looks like it could be best.
            if (score > alpha && (reduction > 0 || score < beta)) {
                score = -negamax(b, depth - 1, ply + 1, -beta, -alpha, opp,
                                 nextPending);
            }
        }
        undoSearchMove(b, u);
        ++moveIndex;

        if (score > bestScore) {
            bestScore = score;
            bestMove = m;
        }
        if (score > alpha) alpha = score;
        if (alpha >= beta) {
            // Beta cutoff: remember this quiet move as a killer / history move.
            if (ply < MAX_PLY) {
                killers[ply][1] = killers[ply][0];
                killers[ply][0] = m;
            }
            historyTable[m.row][m.col] += depth * depth;
            break;
        }
    }

    HashFlag flag = HashFlag::EXACT;
    if (bestScore <= originalAlpha)      flag = HashFlag::ALPHA;
    else if (bestScore >= beta)          flag = HashFlag::BETA;
    tt.store(key, depth, bestScore, flag, bestMove);

    return bestScore;
}

// --- Root: iterative deepening ------------------------------------------------

Point GomokuAI::searchBestMove(GameEngine& engine, Cell aiColor) {
    startTime = std::chrono::steady_clock::now();
    nodeCount = 0;

    for (int i = 0; i < MAX_PLY; ++i) killers[i][0] = killers[i][1] = {-1, -1};
    for (int r = 0; r < BOARD_SIZE; ++r)
        for (int c = 0; c < BOARD_SIZE; ++c)
            historyTable[r][c] = 0;

    // Work on a private copy of the board so the live game is never touched.
    Board board = engine.getBoard();
    Cell opp = getOpponent(aiColor);

    // Build the root move list once: only fully-legal moves (double-three rule)
    // so the move we ultimately return is guaranteed playable by the engine.
    std::vector<Point> rawRoot = MoveGenerator::generateCandidateMoves(board);
    std::vector<Point> rootMoves;
    for (const Point& m : rawRoot) {
        if (GomokuRules::isLegalMove(board, m.row, m.col, aiColor))
            rootMoves.push_back(m);
    }
    if (rootMoves.empty()) {
        // A neighbourhood candidate list can be empty on a nearly saturated
        // board. Fall back to a complete legal scan before returning a point.
        for (int r = 0; r < BOARD_SIZE; ++r)
            for (int c = 0; c < BOARD_SIZE; ++c)
                if (GomokuRules::isLegalMove(board, r, c, aiColor))
                    return Point{r, c};
        return Point{BOARD_SIZE / 2, BOARD_SIZE / 2};
    }

    Point bestMoveOverall = rootMoves[0];
    std::cout << "AI is thinking..." << std::endl;

    try {
        for (int depth = 1; depth <= MAX_PLY; ++depth) {
            // Order root moves, seeding the best move from the previous iteration.
            orderMoves(board, rootMoves, aiColor, 0, bestMoveOverall, ROOT_CAP);

            int alpha = -INF, beta = INF;
            int bestScore = -INF;
            Point bestThisDepth = rootMoves[0];
            int moveIndex = 0;

            for (const Point& m : rootMoves) {
                Undo u;
                applySearchMove(board, m.row, m.col, aiColor, u);
                int score;
                if (moveIndex == 0) {
                    bool nextPending = GomokuRules::hasFiveInRow(board, aiColor) &&
                                       GomokuRules::isFiveBreakable(board, aiColor);
                    score = -negamax(board, depth - 1, 1, -beta, -alpha,
                                     opp, nextPending);
                } else {
                    // PVS + LMR at the root as well (the PV move from the
                    // previous iteration is already searched first).
                    int reduction = 0;
                    if (depth >= 3 && moveIndex >= 3 && u.nCaps == 0) {
                        reduction = 1 + (moveIndex >= 8 ? 1 : 0);
                    }
                    bool nextPending = GomokuRules::hasFiveInRow(board, aiColor) &&
                                       GomokuRules::isFiveBreakable(board, aiColor);
                    score = -negamax(board, depth - 1 - reduction, 1,
                                     -alpha - 1, -alpha, opp, nextPending);
                    if (score > alpha) {
                        score = -negamax(board, depth - 1, 1, -beta, -alpha,
                                         opp, nextPending);
                    }
                }
                undoSearchMove(board, u);
                ++moveIndex;

                if (score > bestScore) {
                    bestScore = score;
                    bestThisDepth = m;
                }
                if (score > alpha) alpha = score;
            }

            // Completed this depth without timing out: commit the result.
            bestMoveOverall = bestThisDepth;
            std::cout << "[Time: " << getElapsedTime() << " ms] Depth " << depth
                      << " done. Best: (" << bestMoveOverall.row << ","
                      << bestMoveOverall.col << ") Score: " << bestScore
                      << " Nodes: " << nodeCount << std::endl;

            // Forced win/loss found: no point searching deeper.
            if (bestScore >= WIN_SCORE || bestScore <= -WIN_SCORE) break;
            // Otherwise keep deepening; if the next depth runs over budget the
            // TimeOutException simply returns this (already committed) result.
        }
    } catch (const TimeOutException&) {
        std::cout << "[Time: " << getElapsedTime() << " ms] Time limit reached. "
                  << "Using best move from last completed depth." << std::endl;
    }

    return bestMoveOverall;
}
