#include "GameEngine.hpp"

GameEngine::GameEngine() : gameOver(false), winner(EMPTY), winReason("") {}

const Board& GameEngine::getBoard() const { return board; }
bool GameEngine::isGameOver() const { return gameOver; }
Cell GameEngine::getWinner() const { return winner; }
std::string GameEngine::getWinReason() const { return winReason; }
bool GameEngine::hasLastMove() const { return !history.empty(); }
Point GameEngine::getLastMove() const {
    if (history.empty()) return Point{-1, -1};
    return history.back().placed;
}

MoveResult GameEngine::applyMove(int r, int c, Cell color) {
    if (gameOver) {
        throw std::invalid_argument("Game is already over.");
    }
    if (color != BLACK && color != WHITE) {
        throw std::invalid_argument("A move must use Black or White.");
    }

    // 1. Validate Move
    if (!GomokuRules::isLegalMove(board, r, c, color)) {
        if (board.inBounds(r, c) && board.getCell(r, c) == EMPTY && GomokuRules::isDoubleThree(board, r, c, color)) {
            throw std::invalid_argument("Forbidden Move: Double-Three!");
        }
        throw std::invalid_argument("Invalid move.");
    }

    // 2. Record Move Result
    MoveResult mv;
    mv.placed = {r, c};
    mv.color = color;

    // 3. Apply Stone
    board.setCell(r, c, color);

    // 4. Process Captures
    mv.captured = GomokuRules::findCapturedStones(board, r, c, color);
    for (const auto& p : mv.captured) {
        board.setCell(p.row, p.col, EMPTY); // Remove from board
    }
    board.addCaptures(color, static_cast<int>(mv.captured.size())); // Update score

    // 5. Update History
    history.push_back(mv);

    // 6. Check End-Game States
    checkWinConditions(color);

    return mv;
}

MoveResult GameEngine::undoMove() {
    if (history.empty()) return MoveResult{ {-1, -1}, EMPTY, {} };

    MoveResult lastMove = history.back();
    history.pop_back();

    // 1. Remove the placed stone
    board.setCell(lastMove.placed.row, lastMove.placed.col, EMPTY);

    // 2. Restore captured stones
    Cell oppColor = getOpponent(lastMove.color);
    for (const auto& p : lastMove.captured) {
        board.setCell(p.row, p.col, oppColor);
    }
    
    // 3. Revert score
    board.removeCaptures(lastMove.color, lastMove.captured.size());

    // 4. Reset game over flags (in case the undone move was a winning move)
    gameOver = false;
    winner = EMPTY;
    winReason = "";

    return lastMove;
}

void GameEngine::checkWinConditions(Cell lastColor) {
    Cell oppColor = getOpponent(lastColor);

    // Condition 1: Win by 10 captures
    if (GomokuRules::hasCaptureWin(board, lastColor)) {
        gameOver = true;
        winner = lastColor;
        winReason = (lastColor == BLACK ? "Black" : "White") + std::string(" wins by 10 captures!");
        return;
    }

    // Condition 2: Opponent's 5-in-a-row survived
    // (If the last player didn't break it on this turn, the opponent wins)
    if (GomokuRules::hasFiveInRow(board, oppColor)) {
        gameOver = true;
        winner = oppColor;
        winReason = (oppColor == BLACK ? "Black" : "White") + std::string(" wins! (5-in-a-row was not broken)");
        return;
    }

    // Condition 3: Last player created a 5-in-a-row
    if (GomokuRules::hasFiveInRow(board, lastColor)) {
        if (!GomokuRules::isFiveBreakable(board, lastColor)) {
            gameOver = true;
            winner = lastColor;
            winReason = (lastColor == BLACK ? "Black" : "White") + std::string(" wins with 5-in-a-row!");
            return;
        }
        // If it is breakable, the game continues (we don't set gameOver = true)
    }

    if (board.isFull()) {
        gameOver = true;
        winner = EMPTY;
        winReason = "Draw - the board is full.";
    }
}