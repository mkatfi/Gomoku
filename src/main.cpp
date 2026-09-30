#include "src/gui/GameWindow.hpp"

int main() {
    Zobrist::init(); // Initialize Zobrist keys before anything else
    GameWindow app;
    app.run();
    return 0;
}