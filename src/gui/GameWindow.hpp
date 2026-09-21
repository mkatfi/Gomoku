#ifndef GAMEWINDOW_HPP
#define GAMEWINDOW_HPP

#include <SFML/Graphics.hpp>
#include "Renderer.hpp"
#include "InputHandler.hpp"
#include "../game/GameSession.hpp"

class GameWindow {
public:
    GameWindow();
    void run();

private:
    // Gamer-style flow: Title -> Mode -> (AI setup if vs AI) -> Playing.
    enum class AppState { Title, Mode, AISetup, Playing };

    sf::RenderWindow window;
    Renderer renderer;
    InputHandler inputHandler;
    GameSession session;
    AppState state = AppState::Title;

    // ===== BONUS (controls help): overlay toggled by the on-screen button =====
    bool showHelp = false;

    // ===== BONUS (AI difficulty selector) + color choice, set on the menu =====
    int aiTimeMs = 250;
    std::string difficultyLabel = "Medium";
    Cell humanColor = BLACK; // AI plays the opposite color

    void startGame(bool vsAI, Cell aiColor);

    void processEvents();
    void onTitleEvent(const sf::Event& event, const sf::Vector2f& click, bool clicked);
    void onModeEvent(const sf::Event& event, const sf::Vector2f& click, bool clicked);
    void onAISetupEvent(const sf::Event& event, const sf::Vector2f& click, bool clicked);
    void onPlayingEvent(const sf::Event& event, const sf::Vector2f& click, bool clicked);
};

#endif // GAMEWINDOW_HPP
