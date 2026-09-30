#include "Renderer.hpp"
#include "GuiConstants.hpp"
#include "../engine/GomokuRules.hpp"
#include <algorithm>
#include <iostream>

// ---------------------------------------------------------------------------
//  Dark-arcade palette (theme-independent). These colors carry the game's
//  visual identity regardless of the light/dark board toggle below:
//  a deep charcoal frame, a crimson vs. warm-cream player pairing, and a
//  single gold accent used sparingly for emphasis.
// ---------------------------------------------------------------------------
namespace {
    const sf::Color BG_DARK    (13, 17, 23);      // outermost window background
    const sf::Color PANEL_BG   (22, 27, 34);      // side panel / button base / overlays
    const sf::Color PANEL_LINE (255, 183, 3, 70);  // faint gold panel borders

    const sf::Color ACCENT      (255, 183, 3);    // gold accent (headings, highlights)
    const sf::Color ACCENT_SOFT (255, 183, 3, 40);

    const sf::Color INK        (245, 245, 245);   // primary text
    const sf::Color INK_DIM    (156, 163, 175);   // secondary / muted text

    const sf::Color P1_COLOR   (230, 57, 70);     // Player 1 stones - crimson
    const sf::Color P2_COLOR   (241, 241, 230);   // Player 2 stones - warm cream

    const sf::Color GOOD       (110, 217, 156);   // "playing" status
    const sf::Color WARN       (255, 107, 107);   // invalid-move / danger text

    const sf::Color HINT       (94, 209, 168, 150); // move-suggestion overlay

    const float CENTER_X = gui::WINDOW_WIDTH / 2.0f;
}

Renderer::Renderer() {
    applyTheme(); // initialise the arcade-dark palette

    // Try a bundled font first (keeps the repo self-contained), then fall back to
    // common system fonts so the program runs on both Linux and macOS.
    const char* candidates[] = {
        "assets/font.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/TTF/DejaVuSans.ttf",
        "/System/Library/Fonts/Supplemental/Arial.ttf",
        "/Library/Fonts/Arial.ttf",
        "/System/Library/Fonts/Geneva.ttf",
    };
    bool loaded = false;
    for (const char* path : candidates) {
        if (font.openFromFile(path)) { loaded = true; break; }
    }
    if (!loaded) {
        std::cerr << "Warning: no usable font found! Text will not render." << std::endl;
    }
}

// Board themes change the surface and grid contrast; the charcoal frame and
// HUD remain consistent so player/status colors keep the same meaning.
void Renderer::applyTheme() {
    if (darkMode) {
        boardColor     = sf::Color(52, 36, 26);    // near-black walnut
        boardShadow    = sf::Color(0, 0, 0, 150);
        lineColor      = sf::Color(198, 168, 120);  // warm gold-tan grid
        stoneOutlineP1 = sf::Color(96, 16, 22);
        stoneOutlineP2 = sf::Color(150, 140, 125);
    } else {
        boardColor     = sf::Color(196, 154, 106);  // lighter parchment wood
        boardShadow    = sf::Color(0, 0, 0, 70);
        lineColor      = sf::Color(64, 42, 26);
        stoneOutlineP1 = sf::Color(120, 20, 26);
        stoneOutlineP2 = sf::Color(120, 110, 96);
    }
}

void Renderer::toggleTheme() {
    darkMode = !darkMode;
    applyTheme();
}

void Renderer::drawText(sf::RenderWindow& window, const std::string& str,
                        float x, float y, unsigned size, sf::Color color, bool bold) {
    sf::Text t(font, str, size);
    t.setFillColor(color);
    if (bold) t.setStyle(sf::Text::Bold);
    t.setPosition({x, y});
    window.draw(t);
}

void Renderer::drawCenteredText(sf::RenderWindow& window, const std::string& str,
                                float centerX, float y, unsigned size, sf::Color color, bool bold) {
    sf::Text t(font, str, size);
    t.setFillColor(color);
    if (bold) t.setStyle(sf::Text::Bold);
    sf::FloatRect b = t.getLocalBounds();
    t.setPosition({centerX - (b.size.x / 2.0f) - b.position.x, y});
    window.draw(t);
}

bool Renderer::isHover(sf::RenderWindow& window, const sf::FloatRect& rect) const {
    sf::Vector2f m = window.mapPixelToCoords(sf::Mouse::getPosition(window));
    return rect.contains(m);
}

void Renderer::drawPanel(sf::RenderWindow& window, const sf::FloatRect& rect,
                         sf::Color fill, sf::Color outline, float outlineThickness) {
    sf::RectangleShape box(rect.size);
    box.setPosition(rect.position);
    box.setFillColor(fill);
    box.setOutlineColor(outline);
    box.setOutlineThickness(outlineThickness);
    window.draw(box);
}

// ---------------------------------------------------------------------------
//  Buttons: a flat gold-on-charcoal arcade button with a drop shadow and a
//  brighter gold fill on hover/selected so state is unmistakable.
// ---------------------------------------------------------------------------
void Renderer::drawButton(sf::RenderWindow& window, const sf::FloatRect& rect,
                          const std::string& label, bool hovered, bool selected) {
    bool hot = hovered || selected;

    sf::RectangleShape shadow(rect.size);
    shadow.setPosition({rect.position.x + 3.f, rect.position.y + 4.f});
    shadow.setFillColor(sf::Color(0, 0, 0, 120));
    window.draw(shadow);

    sf::RectangleShape box(rect.size);
    box.setPosition(rect.position);
    box.setFillColor(hot ? ACCENT : PANEL_BG);
    box.setOutlineColor(hot ? sf::Color(255, 210, 90) : sf::Color(255, 183, 3, 110));
    box.setOutlineThickness(hovered ? 3.f : 2.f);
    window.draw(box);

    sf::Color labelColor = hot ? sf::Color(18, 14, 8) : INK;
    sf::Text t(font, label, 21);
    t.setFillColor(labelColor);
    t.setStyle(sf::Text::Bold);
    sf::FloatRect b = t.getLocalBounds();
    t.setPosition({rect.position.x + (rect.size.x - b.size.x) / 2.0f - b.position.x,
                   rect.position.y + (rect.size.y - b.size.y) / 2.0f - b.position.y});
    window.draw(t);
}

void Renderer::drawMenuBackground(sf::RenderWindow& window) {
    // Subtle vertical gradient instead of a flat clear, for a cinematic feel.
    sf::VertexArray grad(sf::PrimitiveType::TriangleStrip, 4);
    sf::Color top(17, 20, 27);
    sf::Color bottom(8, 10, 14);
    float w = (float)gui::WINDOW_WIDTH, h = (float)gui::WINDOW_HEIGHT;
    grad[0].position = {0.f, 0.f}; grad[0].color = top;
    grad[1].position = {w,   0.f}; grad[1].color = top;
    grad[2].position = {0.f, h  }; grad[2].color = bottom;
    grad[3].position = {w,   h  }; grad[3].color = bottom;
    window.draw(grad);

    sf::RectangleShape bar({w, 3.0f});
    bar.setPosition({0.0f, 150.0f});
    bar.setFillColor(sf::Color(ACCENT.r, ACCENT.g, ACCENT.b, 130));
    window.draw(bar);
}

// ---------------------------------------------------------------------------
//  Button layout (single source of truth) - centered on the full window
//  width, since menu screens don't show the in-game HUD panel.
// ---------------------------------------------------------------------------
sf::FloatRect Renderer::btnTitleStart()  { return {{CENTER_X - 190.f, 520.f}, {380.f, 74.f}}; }
sf::FloatRect Renderer::btnModeAI()      { return {{CENTER_X - 210.f, 300.f}, {420.f, 76.f}}; }
sf::FloatRect Renderer::btnModeHotseat() { return {{CENTER_X - 210.f, 400.f}, {420.f, 76.f}}; }
sf::FloatRect Renderer::btnModeBack()    { return {{CENTER_X - 150.f, 540.f}, {300.f, 50.f}}; }
sf::FloatRect Renderer::btnDiffEasy()    { return {{CENTER_X - 305.f, 300.f}, {190.f, 64.f}}; }
sf::FloatRect Renderer::btnDiffMedium()  { return {{CENTER_X - 95.f,  300.f}, {190.f, 64.f}}; }
sf::FloatRect Renderer::btnDiffHard()    { return {{CENTER_X + 115.f, 300.f}, {190.f, 64.f}}; }
sf::FloatRect Renderer::btnColorBlack()  { return {{CENTER_X - 275.f, 430.f}, {260.f, 64.f}}; }
sf::FloatRect Renderer::btnColorWhite()  { return {{CENTER_X + 15.f,  430.f}, {260.f, 64.f}}; }
sf::FloatRect Renderer::btnAIStart()     { return {{CENTER_X - 210.f, 560.f}, {420.f, 74.f}}; }
sf::FloatRect Renderer::btnAIBack()      { return {{CENTER_X - 110.f, 656.f}, {220.f, 46.f}}; }
sf::FloatRect Renderer::btnHelp()        { return {{(float)gui::BOARD_AREA + 26.f, 16.f}, {(float)gui::HUD_WIDTH - 52.f, 32.f}}; }

// ---------------------------------------------------------------------------
//  Screen: Title
// ---------------------------------------------------------------------------
void Renderer::renderTitle(sf::RenderWindow& window) {
    drawMenuBackground(window);

    drawCenteredText(window, "GOMOKU", CENTER_X + 3, 73, 84, sf::Color(0, 0, 0, 160), true);
    drawCenteredText(window, "GOMOKU", CENTER_X, 70, 84, ACCENT, true);
    drawCenteredText(window, "Pente variant  -  Min-Max AI", CENTER_X, 175, 22, INK_DIM);

    drawButton(window, btnTitleStart(), "START GAME", isHover(window, btnTitleStart()), false);
    drawCenteredText(window, "Click START or press Enter", CENTER_X, 620, 18, INK_DIM);
    drawCenteredText(window, "[T] toggle Dark / Light board theme", CENTER_X, 650, 16, INK_DIM);
    window.display();
}

// ---------------------------------------------------------------------------
//  Screen: Mode selection
// ---------------------------------------------------------------------------
void Renderer::renderModeSelect(sf::RenderWindow& window) {
    drawMenuBackground(window);
    drawCenteredText(window, "SELECT MODE", CENTER_X, 80, 48, ACCENT, true);

    drawButton(window, btnModeAI(),      "Play vs AI",          isHover(window, btnModeAI()), false);
    drawButton(window, btnModeHotseat(), "Hotseat (2 Players)", isHover(window, btnModeHotseat()), false);
    drawButton(window, btnModeBack(),    "< Back",              isHover(window, btnModeBack()), false);
    window.display();
}

// ---------------------------------------------------------------------------
//  Screen: AI setup (difficulty + your color)  [BONUS: AI difficulty selector]
// ---------------------------------------------------------------------------
void Renderer::renderAISetup(sf::RenderWindow& window, const std::string& difficulty, Cell humanColor) {
    drawMenuBackground(window);
    drawCenteredText(window, "AI SETUP", CENTER_X, 80, 48, ACCENT, true);

    drawCenteredText(window, "Difficulty", CENTER_X, 255, 24, INK);
    drawButton(window, btnDiffEasy(),   "Easy",   isHover(window, btnDiffEasy()),   difficulty == "Easy");
    drawButton(window, btnDiffMedium(), "Medium", isHover(window, btnDiffMedium()), difficulty == "Medium");
    drawButton(window, btnDiffHard(),   "Hard",   isHover(window, btnDiffHard()),   difficulty == "Hard");

    drawCenteredText(window, "You play as", CENTER_X, 388, 24, INK);
    drawButton(window, btnColorBlack(), "Player 1 (1st)", isHover(window, btnColorBlack()), humanColor == BLACK);
    drawButton(window, btnColorWhite(), "Player 2 (2nd)", isHover(window, btnColorWhite()), humanColor == WHITE);

    drawButton(window, btnAIStart(), "START GAME", isHover(window, btnAIStart()), false);
    drawButton(window, btnAIBack(),  "< Back",     isHover(window, btnAIBack()), false);
    window.display();
}

// ---------------------------------------------------------------------------
//  Screen: in-game
// ---------------------------------------------------------------------------
void Renderer::render(sf::RenderWindow& window, const GameSession& session, bool showHelp) {
    drawBackground(window);
    drawBoard(window);
    drawGrid(window);
    drawStones(window, session);
    drawLastMove(window, session);
    drawSuggestion(window, session);
    drawHoverPreview(window, session);
    drawHUD(window, session);
    drawHelpButton(window);
    if (showHelp) drawHelpOverlay(window);
    window.display();
}

void Renderer::drawBackground(sf::RenderWindow& window) {
    window.clear(BG_DARK);
}

void Renderer::drawBoard(sf::RenderWindow& window) {
    // Drop shadow behind the wood panel for a bit of depth.
    sf::RectangleShape shadow({(float)gui::BOARD_AREA, (float)gui::BOARD_AREA});
    shadow.setPosition({4.f, 5.f});
    shadow.setFillColor(boardShadow);
    window.draw(shadow);

    sf::RectangleShape panel({(float)gui::BOARD_AREA, (float)gui::BOARD_AREA});
    panel.setPosition({0.f, 0.f});
    panel.setFillColor(boardColor);
    panel.setOutlineColor(ACCENT);
    panel.setOutlineThickness(2.f);
    window.draw(panel);
}

// Shows whose turn it is by tinting the HUD's active player row (see
// drawHUD). Kept as a no-op placeholder here would be dead code, so the
// on-board turn readout was folded directly into the HUD panel instead.

// ===== BONUS (controls help): clickable button that opens the overlay =====
void Renderer::drawHelpButton(sf::RenderWindow& window) {
    drawButton(window, btnHelp(), "? CONTROLS", isHover(window, btnHelp()), false);
}

void Renderer::drawHelpOverlay(sf::RenderWindow& window) {
    sf::RectangleShape dim({(float)gui::WINDOW_WIDTH, (float)gui::WINDOW_HEIGHT});
    dim.setFillColor(sf::Color(0, 0, 0, 190));
    window.draw(dim);

    sf::FloatRect panel({(float)gui::BOARD_AREA / 2.f - 240.f, 210.f}, {480.f, 380.f});
    drawPanel(window, panel, PANEL_BG, ACCENT, 2.f);

    float cx = panel.position.x + panel.size.x / 2.f;
    drawCenteredText(window, "CONTROLS", cx, 235, 34, ACCENT, true);

    float x = panel.position.x + 36.f;
    float y = 300.f;
    auto line = [&](const std::string& s) { drawText(window, s, x, y, 20, INK); y += 40.f; };
    line("Left click  -  place a stone");
    line("H  -  suggest a move (hint)");
    line("U  -  undo        Y  -  redo");
    line("T  -  toggle Dark / Light theme");
    line("R  -  back to main menu");
    drawCenteredText(window, "Click ? CONTROLS again to close", cx, 545, 16, INK_DIM);
}

void Renderer::drawGrid(sf::RenderWindow& window) {
    // Grid lines drawn as thin filled rectangles (rather than 1px sf::Lines)
    // so they stay crisp and clearly visible against the wood panel.
    const float thickness = 1.6f;
    for (int i = 0; i < BOARD_SIZE; i++) {
        sf::RectangleShape vLine({thickness, (float)gui::BOARD_PIXELS});
        vLine.setOrigin({thickness / 2.f, 0.f});
        vLine.setPosition({(float)gui::MARGIN + i * gui::CELL_SIZE, (float)gui::MARGIN});
        vLine.setFillColor(lineColor);
        window.draw(vLine);

        sf::RectangleShape hLine({(float)gui::BOARD_PIXELS, thickness});
        hLine.setOrigin({0.f, thickness / 2.f});
        hLine.setPosition({(float)gui::MARGIN, (float)gui::MARGIN + i * gui::CELL_SIZE});
        hLine.setFillColor(lineColor);
        window.draw(hLine);
    }

    // Star points (Hoshi)
    const int hoshi[] = {3, 9, 15};
    for (int r : hoshi) {
        for (int c : hoshi) {
            sf::CircleShape dot(4.f);
            dot.setFillColor(lineColor);
            dot.setOrigin({4.f, 4.f});
            dot.setPosition({(float)gui::MARGIN + c * gui::CELL_SIZE, (float)gui::MARGIN + r * gui::CELL_SIZE});
            window.draw(dot);
        }
    }
}

void Renderer::drawStones(sf::RenderWindow& window, const GameSession& session) {
    const Board& board = session.getEngine().getBoard();

    for (int r = 0; r < BOARD_SIZE; r++) {
        for (int c = 0; c < BOARD_SIZE; c++) {
            Cell cell = board.getCell(r, c);
            if (cell == EMPTY) continue;

            sf::Vector2f pos{(float)gui::MARGIN + c * gui::CELL_SIZE,
                             (float)gui::MARGIN + r * gui::CELL_SIZE};
            float radius = (float)gui::STONE_RADIUS;

            // Soft contact shadow under each stone for a bit of depth.
            sf::CircleShape shadow(radius);
            shadow.setOrigin({radius, radius});
            shadow.setPosition({pos.x + 1.5f, pos.y + 2.f});
            shadow.setFillColor(sf::Color(0, 0, 0, 90));
            window.draw(shadow);

            sf::CircleShape stone(radius);
            stone.setOrigin({radius, radius});
            stone.setPosition(pos);

            if (cell == BLACK) {
                stone.setFillColor(P1_COLOR);
                stone.setOutlineColor(stoneOutlineP1);
            } else {
                stone.setFillColor(P2_COLOR);
                stone.setOutlineColor(stoneOutlineP2);
            }
            stone.setOutlineThickness(1.5f);
            window.draw(stone);

            // Small highlight for a polished, slightly glossy bead look.
            sf::CircleShape shine(radius * 0.32f);
            shine.setOrigin({radius * 0.32f, radius * 0.32f});
            shine.setPosition({pos.x - radius * 0.32f, pos.y - radius * 0.32f});
            shine.setFillColor(sf::Color(255, 255, 255, cell == BLACK ? 55 : 90));
            window.draw(shine);
        }
    }
}

void Renderer::drawLastMove(sf::RenderWindow& window, const GameSession& session) {
    const GameEngine& engine = session.getEngine();
    if (!engine.hasLastMove()) return;

    Point lm = engine.getLastMove();
    sf::Vector2f pos{(float)gui::MARGIN + lm.col * gui::CELL_SIZE,
                     (float)gui::MARGIN + lm.row * gui::CELL_SIZE};
    float r = (float)gui::STONE_RADIUS;

    // Soft outer glow + crisp inner ring in the gold accent color.
    sf::CircleShape glow(r + 6.f);
    glow.setOrigin({r + 6.f, r + 6.f});
    glow.setPosition(pos);
    glow.setFillColor(sf::Color::Transparent);
    glow.setOutlineColor(ACCENT_SOFT);
    glow.setOutlineThickness(4.f);
    window.draw(glow);

    sf::CircleShape ring(r + 3.f);
    ring.setOrigin({r + 3.f, r + 3.f});
    ring.setPosition(pos);
    ring.setFillColor(sf::Color::Transparent);
    ring.setOutlineColor(ACCENT);
    ring.setOutlineThickness(2.f);
    window.draw(ring);
}

// Highlights the AI's suggested move (the [H] hint), when one is active.
void Renderer::drawSuggestion(sf::RenderWindow& window, const GameSession& session) {
    auto hint = session.getSuggestion();
    if (!hint) return;

    sf::Vector2f pos{(float)gui::MARGIN + hint->col * gui::CELL_SIZE,
                     (float)gui::MARGIN + hint->row * gui::CELL_SIZE};
    float r = (float)gui::STONE_RADIUS;

    sf::CircleShape ring(r);
    ring.setOrigin({r, r});
    ring.setPosition(pos);
    ring.setFillColor(sf::Color(HINT.r, HINT.g, HINT.b, 60));
    ring.setOutlineColor(HINT);
    ring.setOutlineThickness(2.5f);
    window.draw(ring);
}

// ===== Valid board-position preview: a translucent stone under the cursor =====
void Renderer::drawHoverPreview(sf::RenderWindow& window, const GameSession& session) {
    if (session.isGameOver() || session.isAITurn()) return;

    sf::Vector2f m = window.mapPixelToCoords(sf::Mouse::getPosition(window));
    int col = (int)((m.x - gui::MARGIN + gui::CELL_SIZE / 2) / gui::CELL_SIZE);
    int row = (int)((m.y - gui::MARGIN + gui::CELL_SIZE / 2) / gui::CELL_SIZE);
    if (row < 0 || row >= BOARD_SIZE || col < 0 || col >= BOARD_SIZE) return;

    const Board& board = session.getEngine().getBoard();
    if (board.getCell(row, col) != EMPTY) return;

    Cell turn = session.getCurrentTurn();
    bool legal = GomokuRules::isLegalMove(board, row, col, turn);
    sf::Color base = legal
        ? ((turn == BLACK) ? P1_COLOR : P2_COLOR)
        : WARN;

    sf::Vector2f pos{(float)gui::MARGIN + col * gui::CELL_SIZE,
                     (float)gui::MARGIN + row * gui::CELL_SIZE};
    float r = (float)gui::STONE_RADIUS;

    sf::CircleShape ghost(r);
    ghost.setOrigin({r, r});
    ghost.setPosition(pos);
    ghost.setFillColor(sf::Color(base.r, base.g, base.b, legal ? 95 : 70));
    ghost.setOutlineColor(sf::Color(base.r, base.g, base.b, legal ? 190 : 230));
    ghost.setOutlineThickness(1.5f);
    window.draw(ghost);

    if (!legal) {
        // A clear cross makes a forbidden double-three preview distinguishable
        // from an ordinary empty intersection before the player clicks.
        sf::VertexArray cross(sf::PrimitiveType::Lines, 4);
        cross[0].position = {pos.x - 5.f, pos.y - 5.f};
        cross[1].position = {pos.x + 5.f, pos.y + 5.f};
        cross[2].position = {pos.x + 5.f, pos.y - 5.f};
        cross[3].position = {pos.x - 5.f, pos.y + 5.f};
        for (std::size_t i = 0; i < 4; ++i)
            cross[i].color = sf::Color(255, 255, 255, 220);
        window.draw(cross);
    }
}

int Renderer::countStonesOnBoard(const GameSession& session) const {
    const Board& board = session.getEngine().getBoard();
    int count = 0;
    for (int r = 0; r < BOARD_SIZE; r++)
        for (int c = 0; c < BOARD_SIZE; c++)
            if (board.getCell(r, c) != EMPTY) count++;
    return count;
}

// ---------------------------------------------------------------------------
//  In-game HUD: a docked right-hand panel with player identity, whose turn
//  it is, move count, status, captures, and a clear winner banner. This is
//  purely a read-out of existing accessors (GameSession / GameEngine /
//  Board) - no gameplay state is touched.
// ---------------------------------------------------------------------------
void Renderer::drawHUD(sf::RenderWindow& window, const GameSession& session) {
    float panelX = (float)gui::BOARD_AREA;
    float panelW = (float)gui::HUD_WIDTH;
    float panelH = (float)gui::WINDOW_HEIGHT;

    drawPanel(window, {{panelX, 0.f}, {panelW, panelH}}, PANEL_BG, PANEL_LINE, 1.f);

    sf::RectangleShape divider({3.f, panelH});
    divider.setPosition({panelX, 0.f});
    divider.setFillColor(ACCENT);
    window.draw(divider);

    const float x = panelX + 26.f;
    const float innerW = panelW - 52.f;
    float y = 64.f; // leave room for the "? CONTROLS" button docked above

    auto rule = [&]() {
        sf::RectangleShape r({innerW, 1.f});
        r.setPosition({x, y});
        r.setFillColor(sf::Color(255, 255, 255, 28));
        window.draw(r);
    };

    drawText(window, "GOMOKU", x, y, 26, ACCENT, true);
    y += 46.f;
    rule();
    y += 20.f;

    const Board& board = session.getEngine().getBoard();
    Cell turn = session.getCurrentTurn();
    bool gameOver = session.isGameOver();
    Cell winner = session.getEngine().getWinner();

    // --- Player identity rows, with the active player clearly highlighted ---
    auto drawPlayerRow = [&](Cell color, const std::string& label, const std::string& swatch,
                              sf::Color dotColor, sf::Color outline) {
        bool active = !gameOver && (turn == color);
        float rowH = 50.f;

        if (active) {
            sf::RectangleShape hi({innerW + 14.f, rowH});
            hi.setPosition({x - 12.f, y - 8.f});
            hi.setFillColor(sf::Color(dotColor.r, dotColor.g, dotColor.b, 26));
            hi.setOutlineColor(dotColor);
            hi.setOutlineThickness(1.5f);
            window.draw(hi);
        }

        sf::CircleShape dot(9.f);
        dot.setOrigin({9.f, 9.f});
        dot.setPosition({x + 10.f, y + 17.f});
        dot.setFillColor(dotColor);
        dot.setOutlineColor(outline);
        dot.setOutlineThickness(1.5f);
        window.draw(dot);

        drawText(window, label, x + 30.f, y - 2.f, 14, INK_DIM, true);
        drawText(window, swatch, x + 30.f, y + 14.f, 17, INK, true);
        y += rowH + 4.f;
    };

    drawPlayerRow(BLACK, "PLAYER 1", "RED",   P1_COLOR, stoneOutlineP1);
    drawPlayerRow(WHITE, "PLAYER 2", "WHITE", P2_COLOR, stoneOutlineP2);

    y += 8.f;
    rule();
    y += 20.f;

    // --- Current turn ---
    drawText(window, "CURRENT TURN", x, y, 13, INK_DIM, true);
    y += 22.f;
    if (!gameOver) {
        std::string turnLabel = (turn == BLACK) ? "PLAYER 1" : "PLAYER 2";
        drawText(window, turnLabel, x, y, 23, (turn == BLACK ? P1_COLOR : P2_COLOR), true);
    } else {
        drawText(window, "-", x, y, 23, INK_DIM, true);
    }
    y += 42.f;

    // --- Move counter ---
    drawText(window, "MOVE", x, y, 13, INK_DIM, true);
    y += 22.f;
    drawText(window, std::to_string(countStonesOnBoard(session)), x, y, 23, INK, true);
    y += 42.f;

    // --- Status ---
    drawText(window, "STATUS", x, y, 13, INK_DIM, true);
    y += 22.f;
    std::string statusWord = !gameOver ? "PLAYING" : (winner == EMPTY ? "DRAW" : "GAME OVER");
    sf::Color statusColor  = !gameOver ? GOOD       : (winner == EMPTY ? INK_DIM : ACCENT);
    drawText(window, statusWord, x, y, 23, statusColor, true);
    y += 42.f;

    rule();
    y += 20.f;

    // --- Capture-to-win progress: five lit segments represent five pairs ---
    drawText(window, "CAPTURES", x, y, 13, INK_DIM, true);
    y += 20.f;
    auto drawCaptureTrack = [&](Cell color, const std::string& label,
                                sf::Color tint, float rowY) {
        int pairs = std::min(5, board.getCaptures(color) / 2);
        drawText(window, label, x, rowY - 2.f, 12, INK_DIM, true);
        drawText(window, std::to_string(pairs) + "/5", x + 27.f, rowY - 2.f,
                 12, tint, true);

        const float trackX = x + 76.f;
        const float segmentW = 25.f;
        const float segmentH = 10.f;
        const float gap = 6.f;
        for (int i = 0; i < 5; ++i) {
            sf::RectangleShape segment({segmentW, segmentH});
            segment.setPosition({trackX + i * (segmentW + gap), rowY});
            segment.setFillColor(i < pairs
                ? tint
                : sf::Color(tint.r, tint.g, tint.b, 35));
            segment.setOutlineColor(sf::Color(tint.r, tint.g, tint.b, 150));
            segment.setOutlineThickness(1.f);
            window.draw(segment);
        }
    };
    drawCaptureTrack(BLACK, "P1", P1_COLOR, y);
    y += 27.f;
    drawCaptureTrack(WHITE, "P2", P2_COLOR, y);
    y += 34.f;

    // --- Mode + AI think time (existing bonus feature, restyled) ---
    std::string modeLine = session.isVsAI() ? "Mode: vs AI" : "Mode: Hotseat";
    drawText(window, modeLine, x, y, 13, INK_DIM);
    y += 18.f;
    drawText(window, "AI last move: " + std::to_string(session.getLastThinkMs()) + " ms",
             x, y, 13, INK_DIM);
    y += 28.f;

    rule();
    y += 16.f;

    // --- Secondary status message (hints / undo / invalid-move feedback) ---
    drawText(window, session.getStatusMsg(), x, y, 13,
             (!gameOver && session.getStatusMsg().rfind("Invalid", 0) == 0) ? WARN : INK_DIM);

    // --- Winner banner: unmistakable when the game ends ---
    if (gameOver) {
        bool draw = (winner == EMPTY);
        sf::Color bannerColor = draw ? INK_DIM : (winner == BLACK ? P1_COLOR : P2_COLOR);
        std::string bannerText = draw ? "DRAW" : (winner == BLACK ? "PLAYER 1 WINS" : "PLAYER 2 WINS");

        float by = panelH - 96.f;
        sf::RectangleShape banner({innerW + 14.f, 66.f});
        banner.setPosition({x - 12.f, by});
        banner.setFillColor(sf::Color(bannerColor.r, bannerColor.g, bannerColor.b, draw ? 22 : 36));
        banner.setOutlineColor(bannerColor);
        banner.setOutlineThickness(2.f);
        window.draw(banner);

        drawCenteredText(window, bannerText, x - 12.f + (innerW + 14.f) / 2.f, by + 20.f, 19,
                          bannerColor, true);
    }
}
