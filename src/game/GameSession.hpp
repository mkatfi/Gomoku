#ifndef GAMESESSION_HPP
#define GAMESESSION_HPP

#include "../engine/GameEngine.hpp"
#include <string>
#include <optional>
#include <vector>

class GameSession {
public:
    GameSession(bool playVsAI = true, Cell aiPlayerColor = WHITE);

    // Restart a fresh game with a new configuration (used by the menu).
    void reset(bool playVsAI, Cell aiPlayerColor);

    //AI Turn handlers

    bool isAITurn() const;
    void handleAITurn();

    // ===== BONUS (Undo / Redo) =====
    // In vs-AI mode these step over a full round (your move + the AI's reply) so
    // control returns to you; in hotseat they step one ply.
    void undo();
    void redo();
    bool canUndo() const;
    bool canRedo() const;

    // ===== BONUS (AI difficulty selector) =====
    // AI strength: per-move time budget in milliseconds.
    void setAITime(int ms);
    int  getAITime() const;

    // Move-suggestion (hint) feature: ask the AI for the best move for whoever
    // is to play right now, without actually playing it.
    void requestSuggestion();
    void clearSuggestion();
    std::optional<Point> getSuggestion() const;

    // Core Interaction
    void handlePlayerMove(int r, int c);

    // Accessors for the GUI
    const GameEngine& getEngine() const;
    Cell getCurrentTurn() const;
    std::string getStatusMsg() const;
    bool isGameOver() const;
    bool isVsAI() const;
    Cell getAIColor() const;          // which color the AI plays (vs-AI mode)
    long long getLastThinkMs() const; // time the AI took on its last search

private:
    GameEngine engine;
    Cell currentTurn;
    std::string statusMsg;
    // AI State
    bool vsAI;
    Cell aiColor;

    // GUI helpers
    long long lastThinkMs = 0;
    std::optional<Point> suggestion;
    int aiTimeMs = 250;                 // BONUS (AI difficulty): medium by default
    std::vector<MoveResult> redoStack;  // BONUS (Redo): moves that were undone

    // Set when a player has no legal move left: the game is a draw and we must
    // stop asking the AI for a move (otherwise it would loop forever).
    bool noLegalMove = false;

    // Internal helpers
    void switchTurn();
    void updateStatusAfterMove();
    Point runSearch(Cell forColor); // shared minimax driver + timing
    bool hasLegalMove(Cell color) const;
};

#endif // GAMESESSION_HPP