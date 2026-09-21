#ifndef GUICONSTANTS_HPP
#define GUICONSTANTS_HPP

#include "../core/Types.hpp"

// ---------------------------------------------------------------------------
//  Layout constants for the dark-arcade UI.
//
//  The play area is a square board framed by MARGIN on every side. A HUD
//  panel of fixed width is docked to the right of the board so the window
//  is: [ board square (BOARD_AREA x BOARD_AREA) ][ HUD_WIDTH ].
//  Only these numbers changed from the previous layout — nothing here
//  affects move validation or coordinate math beyond where pixels are drawn.
// ---------------------------------------------------------------------------
namespace gui {
    const int CELL_SIZE     = 40;
    const int MARGIN        = 46;
    const int BOARD_PIXELS  = (BOARD_SIZE - 1) * CELL_SIZE;
    const int BOARD_AREA    = BOARD_PIXELS + 2 * MARGIN;  // square board region
    const int HUD_WIDTH     = 300;

    const int WINDOW_WIDTH  = BOARD_AREA + HUD_WIDTH;
    const int WINDOW_HEIGHT = BOARD_AREA;

    const int STONE_RADIUS  = CELL_SIZE / 2 - 3;
}

#endif // GUICONSTANTS_HPP
