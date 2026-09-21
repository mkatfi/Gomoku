#include "GameWindow.hpp"
#include "GuiConstants.hpp"

GameWindow::GameWindow()
    : window(sf::VideoMode({(unsigned int)gui::WINDOW_WIDTH, (unsigned int)gui::WINDOW_HEIGHT}), "Gomoku Engine")
{
    window.setFramerateLimit(60);
}

void GameWindow::run() {
    while (window.isOpen()) {
        processEvents();

        switch (state) {
            case AppState::Title:    renderer.renderTitle(window); break;
            case AppState::Mode:     renderer.renderModeSelect(window); break;
            case AppState::AISetup:  renderer.renderAISetup(window, difficultyLabel, humanColor); break;
            case AppState::Playing:
                if (session.isAITurn()) session.handleAITurn(); // AI moves on its turn
                renderer.render(window, session, showHelp);
                break;
        }
    }
}

void GameWindow::startGame(bool vsAI, Cell aiColor) {
    session.reset(vsAI, aiColor);
    session.setAITime(aiTimeMs); // BONUS (AI difficulty): carry the menu choice into the game
    showHelp = false;
    state = AppState::Playing;
}

void GameWindow::processEvents() {
    while (const std::optional event = window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            window.close();
            return;
        }

        // ===== BONUS (Dark / Light theme): [T] works on every screen =====
        if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
            if (key->code == sf::Keyboard::Key::T) {
                renderer.toggleTheme();
                continue;
            }
        }

        // Resolve a left-click into world coordinates once, for hit-testing.
        bool clicked = false;
        sf::Vector2f click;
        if (const auto* mb = event->getIf<sf::Event::MouseButtonPressed>()) {
            if (mb->button == sf::Mouse::Button::Left) {
                clicked = true;
                click = window.mapPixelToCoords(mb->position);
            }
        }

        switch (state) {
            case AppState::Title:   onTitleEvent(*event, click, clicked); break;
            case AppState::Mode:    onModeEvent(*event, click, clicked); break;
            case AppState::AISetup: onAISetupEvent(*event, click, clicked); break;
            case AppState::Playing: onPlayingEvent(*event, click, clicked); break;
        }
    }
}

void GameWindow::onTitleEvent(const sf::Event& event, const sf::Vector2f& click, bool clicked) {
    const auto* key = event.getIf<sf::Event::KeyPressed>();
    bool enter = key && (key->code == sf::Keyboard::Key::Enter);
    if (enter || (clicked && Renderer::btnTitleStart().contains(click))) {
        state = AppState::Mode;
    }
}

void GameWindow::onModeEvent(const sf::Event& event, const sf::Vector2f& click, bool clicked) {
    const auto* key = event.getIf<sf::Event::KeyPressed>();
    if (key && key->code == sf::Keyboard::Key::Escape) { state = AppState::Title; return; }
    if (!clicked) return;

    if (Renderer::btnModeAI().contains(click))           state = AppState::AISetup;
    else if (Renderer::btnModeHotseat().contains(click)) startGame(false, EMPTY);
    else if (Renderer::btnModeBack().contains(click))    state = AppState::Title;
}

void GameWindow::onAISetupEvent(const sf::Event& event, const sf::Vector2f& click, bool clicked) {
    using K = sf::Keyboard::Key;
    // Keyboard shortcuts for difficulty still work.
    if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
        switch (key->code) {
            case K::E: aiTimeMs = 100; difficultyLabel = "Easy";   break;
            case K::M: aiTimeMs = 250; difficultyLabel = "Medium"; break;
            case K::D: aiTimeMs = 420; difficultyLabel = "Hard";   break;
            case K::Escape: state = AppState::Mode; break;
            default: break;
        }
        return;
    }
    if (!clicked) return;

    // ===== BONUS (AI difficulty selector) =====
    if (Renderer::btnDiffEasy().contains(click))        { aiTimeMs = 100; difficultyLabel = "Easy"; }
    else if (Renderer::btnDiffMedium().contains(click)) { aiTimeMs = 250; difficultyLabel = "Medium"; }
    else if (Renderer::btnDiffHard().contains(click))   { aiTimeMs = 420; difficultyLabel = "Hard"; }
    else if (Renderer::btnColorBlack().contains(click)) { humanColor = BLACK; }
    else if (Renderer::btnColorWhite().contains(click)) { humanColor = WHITE; }
    else if (Renderer::btnAIBack().contains(click))     { state = AppState::Mode; }
    else if (Renderer::btnAIStart().contains(click))    { startGame(true, getOpponent(humanColor)); }
}

void GameWindow::onPlayingEvent(const sf::Event& event, const sf::Vector2f& click, bool clicked) {
    using K = sf::Keyboard::Key;

    if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
        switch (key->code) {
            case K::R: state = AppState::Title; break;                              // back to menu
            case K::H: if (!session.isAITurn()) session.requestSuggestion(); break; // suggestion
            // ===== BONUS (Undo / Redo) =====
            case K::U: if (session.canUndo()) session.undo(); break;
            case K::Y: if (session.canRedo()) session.redo(); break;
            default: break;
        }
        return;
    }

    if (!clicked) return;

    // ===== BONUS (controls help): the on-screen button toggles the overlay =====
    if (Renderer::btnHelp().contains(click)) { showHelp = !showHelp; return; }
    if (showHelp) return; // overlay is modal: ignore board clicks while it's open

    // Place a stone (ignored during the AI's turn or after the game ends).
    if (!session.isGameOver() && !session.isAITurn()) {
        auto pt = inputHandler.getBoardCoordinates(click.x, click.y);
        if (pt) session.handlePlayerMove(pt->row, pt->col);
    }
}
