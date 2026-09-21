#ifndef INPUTHANDLER_HPP
#define INPUTHANDLER_HPP

#include "../core/Types.hpp"
#include <optional>

class InputHandler {
public:
    std::optional<Point> getBoardCoordinates(float mouseX, float mouseY) const;
};

#endif // INPUTHANDLER_HPP