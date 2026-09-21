#include "InputHandler.hpp"
#include "GuiConstants.hpp"

std::optional<Point> InputHandler::getBoardCoordinates(float mouseX, float mouseY) const {
    int col = (mouseX - gui::MARGIN + gui::CELL_SIZE / 2) / gui::CELL_SIZE;
    int row = (mouseY - gui::MARGIN + gui::CELL_SIZE / 2) / gui::CELL_SIZE;

    if (row >= 0 && row < BOARD_SIZE && col >= 0 && col < BOARD_SIZE) {
        return Point{row, col};
    }
    
    return std::nullopt;
}