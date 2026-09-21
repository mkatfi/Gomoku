#include "GameSession.hpp"
#include "../engine/Rules.hpp"
#include "../ai/AI.hpp"
#include <chrono>

GameSession::GameSession(bool playVsAI , Cell aiPlayerColor ) : currentTurn(BLACK), statusMsg("Black's turn"), vsAI(playVsAI), aiColor(aiPlayerColor) {}

void GameSession::reset(bool playVsAI, Cell aiPlayerColor) {
    engine = GameEngine();
    currentTurn = BLACK;
    statusMsg = "Black's turn";
    vsAI = playVsAI;
    aiColor = aiPlayerColor;
    lastThinkMs = 0;
    suggestion.reset();
    noLegalMove = false;
    redoStack.clear();
}

// ===== BONUS (AI difficulty selector) =====
void GameSession::setAITime(int ms) { aiTimeMs = ms; }
int  GameSession::getAITime() const { return aiTimeMs; }

const GameEngine& GameSession::getEngine() const { return engine; }
Cell GameSession::getCurrentTurn() const { return currentTurn; }
std::string GameSession::getStatusMsg() const { return statusMsg; }
bool GameSession::isGameOver() const { return engine.isGameOver() || noLegalMove; }

// True if `color` has at least one legal move anywhere on the board.
bool GameSession::hasLegalMove(Cell color) const {
    const Board& b = engine.getBoard();
    for (int r = 0; r < BOARD_SIZE; ++r)
        for (int c = 0; c < BOARD_SIZE; ++c)
            if (b.getCell(r, c) == EMPTY && Rules::isLegalMove(b, r, c, color))
                return true;
    return false;
}
bool GameSession::isVsAI() const { return vsAI; }
Cell GameSession::getAIColor() const { return aiColor; }
long long GameSession::getLastThinkMs() const { return lastThinkMs; }

std::optional<Point> GameSession::getSuggestion() const { return suggestion; }
void GameSession::clearSuggestion() { suggestion.reset(); }

// Runs the AI search for the given color and records how long it took.
Point GameSession::runSearch(Cell forColor) {
    AI ai;
    ai.setTimeLimit(aiTimeMs); // BONUS (AI difficulty): apply chosen time budget
    auto start = std::chrono::steady_clock::now();
    Point move = ai.getBestMove(engine, forColor);
    auto end = std::chrono::steady_clock::now();
    lastThinkMs = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    return move;
}

void GameSession::requestSuggestion() {
    if (engine.isGameOver()) return;
    Point hint = runSearch(currentTurn);
    suggestion = hint;
    statusMsg = "Suggested move: (" + std::to_string(hint.row) + ", "
              + std::to_string(hint.col) + ")  [" + std::to_string(lastThinkMs) + " ms]";
}

void GameSession::handlePlayerMove(int r, int c) {
    if (engine.isGameOver()) return;

    try {
        engine.applyMove(r, c, currentTurn);
        suggestion.reset();  // a stone was placed; old hint is stale
        redoStack.clear();   // BONUS (Redo): a fresh move invalidates redo history
        updateStatusAfterMove();
    } catch (const std::invalid_argument& e) {
        // If the move is illegal, catch the engine's error and display it
        statusMsg = std::string("Invalid move: ") + e.what();
    }
}

void GameSession::updateStatusAfterMove() {
    // 1. Did that move end the game?
    if (engine.isGameOver()) {
        statusMsg = engine.getWinReason();
        return;
    }

    // 2. Did that move create a *breakable* 5-in-a-row?
    // (If it was unbreakable, engine.isGameOver() would be true above)
    bool breakableFive = Rules::hasFive(engine.getBoard(), currentTurn);
    std::string oppName = (currentTurn == BLACK) ? "White" : "Black";

    // 3. Hand over to the other player.
    switchTurn();

    // 4. If that player has no legal move at all, the game is a draw. This also
    //    prevents the AI loop from being asked for an impossible move forever.
    if (!hasLegalMove(currentTurn)) {
        noLegalMove = true;
        statusMsg = "Draw - no legal moves remaining.";
        return;
    }

    if (breakableFive) {
        statusMsg = "5-in-a-row formed! " + oppName + " must break it or lose.";
    } else {
        statusMsg = (currentTurn == BLACK) ? "Black's turn" : "White's turn";
    }
}

void GameSession::switchTurn() {
    currentTurn = getOpponent(currentTurn);
}

bool GameSession::isAITurn() const {
    // It's the AI's turn if vsAI is true, the game isn't over, and the current turn matches the AI's color
    return vsAI && !isGameOver() && (currentTurn == aiColor);
}

void GameSession::handleAITurn() {
    if (!isAITurn()) return;

    Point bestMove = runSearch(aiColor);

    // Safety net: if the search could not return a legal move (e.g. a saturated
    // board), declare a draw instead of looping on an impossible move.
    if (!Rules::isLegalMove(engine.getBoard(), bestMove.row, bestMove.col, aiColor)) {
        noLegalMove = true;
        statusMsg = "Draw - no legal moves remaining.";
        return;
    }

    // We can re-use the player move logic because it safely applies the move and switches turns!
    handlePlayerMove(bestMove.row, bestMove.col);
}

// ===== BONUS (Undo / Redo) =====
bool GameSession::canUndo() const { return engine.hasLastMove(); }
bool GameSession::canRedo() const { return !redoStack.empty(); }

void GameSession::undo() {
    int count = vsAI ? 2 : 1; // step over the AI's reply too, in vs-AI mode
    MoveResult last{ {-1, -1}, EMPTY, {} };
    bool any = false;

    for (int i = 0; i < count && engine.hasLastMove(); ++i) {
        last = engine.undoMove();
        redoStack.push_back(last); // pushed AI-move then your-move; redo pops in reverse
        any = true;
    }
    if (!any) return;

    // Whoever made the last move we removed is back on the clock.
    currentTurn = last.color;
    suggestion.reset();
    noLegalMove = false;
    statusMsg = (currentTurn == BLACK) ? "Black's turn (undo)" : "White's turn (undo)";
}

void GameSession::redo() {
    int count = vsAI ? 2 : 1;
    MoveResult last{ {-1, -1}, EMPTY, {} };
    bool any = false;

    for (int i = 0; i < count && !redoStack.empty(); ++i) {
        MoveResult mv = redoStack.back();
        redoStack.pop_back();
        try {
            engine.applyMove(mv.placed.row, mv.placed.col, mv.color);
        } catch (const std::invalid_argument&) {
            redoStack.push_back(mv); // could not replay; leave it
            break;
        }
        last = mv;
        any = true;
    }
    if (!any) return;

    suggestion.reset();
    noLegalMove = false;
    if (engine.isGameOver()) {
        statusMsg = engine.getWinReason();
    } else {
        // After replaying the last move, it is the other player's turn.
        currentTurn = getOpponent(last.color);
        statusMsg = (currentTurn == BLACK) ? "Black's turn (redo)" : "White's turn (redo)";
    }
}