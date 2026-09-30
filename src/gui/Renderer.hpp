#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <SFML/Graphics.hpp>
#include "../game/GameSession.hpp"

class Renderer {
public:
    Renderer();

    // --- Screen renderers ---
    void render(sf::RenderWindow& window, const GameSession& session, bool showHelp);
    void renderTitle(sf::RenderWindow& window);
    void renderModeSelect(sf::RenderWindow& window);
    void renderAISetup(sf::RenderWindow& window, const std::string& difficulty, Cell humanColor);

    // ===== BONUS (Dark / Light theme) =====
    void toggleTheme(); // switch the board between walnut and parchment palettes

    // --- Clickable button rectangles (single source of truth, also hit-tested
    //     by GameWindow). Positions are in window pixels. ---
    static sf::FloatRect btnTitleStart();
    static sf::FloatRect btnModeAI();
    static sf::FloatRect btnModeHotseat();
    static sf::FloatRect btnModeBack();
    static sf::FloatRect btnDiffEasy();
    static sf::FloatRect btnDiffMedium();
    static sf::FloatRect btnDiffHard();
    static sf::FloatRect btnColorBlack();
    static sf::FloatRect btnColorWhite();
    static sf::FloatRect btnAIStart();
    static sf::FloatRect btnAIBack();
    static sf::FloatRect btnHelp();      // in-game "controls" button

private:
    sf::Font font;

    // ===== Board palette: swapped by toggleTheme =====
    bool darkMode = true; // dark arcade palette is the default look
    sf::Color boardColor;     // wood panel behind the grid
    sf::Color boardShadow;    // drop shadow cast by the board panel
    sf::Color lineColor;      // grid lines + hoshi points
    sf::Color stoneOutlineP1; // outline for Player 1 (crimson) stones
    sf::Color stoneOutlineP2; // outline for Player 2 (cream) stones
    void applyTheme();

    // --- Text + widget helpers ---
    void drawText(sf::RenderWindow& window, const std::string& str,
                  float x, float y, unsigned size, sf::Color color, bool bold = false);
    void drawCenteredText(sf::RenderWindow& window, const std::string& str,
                          float centerX, float y, unsigned size, sf::Color color, bool bold = false);
    bool isHover(sf::RenderWindow& window, const sf::FloatRect& rect) const;
    void drawButton(sf::RenderWindow& window, const sf::FloatRect& rect,
                    const std::string& label, bool hovered, bool selected);
    void drawPanel(sf::RenderWindow& window, const sf::FloatRect& rect,
                   sf::Color fill, sf::Color outline, float outlineThickness = 2.f);
    void drawMenuBackground(sf::RenderWindow& window);

    // --- In-game rendering pipeline (see render()) ---
    void drawBackground(sf::RenderWindow& window);
    void drawBoard(sf::RenderWindow& window);
    void drawGrid(sf::RenderWindow& window);
    void drawStones(sf::RenderWindow& window, const GameSession& session);
    void drawLastMove(sf::RenderWindow& window, const GameSession& session);
    void drawHoverPreview(sf::RenderWindow& window, const GameSession& session);
    void drawSuggestion(sf::RenderWindow& window, const GameSession& session);
    void drawHUD(sf::RenderWindow& window, const GameSession& session);
    void drawHelpButton(sf::RenderWindow& window);
    void drawHelpOverlay(sf::RenderWindow& window);

    int countStonesOnBoard(const GameSession& session) const; // for the HUD move counter
};

#endif // RENDERER_HPP
