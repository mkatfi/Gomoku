#include "src/engine/GameEngine.hpp"
#include "src/engine/Rules.hpp"
#include "src/engine/Zobrist.hpp"
#include "src/ai/AI.hpp"
#include <cassert>
#include <iostream>

namespace {
    void testPairCapture() {
        GameEngine game;
        game.applyMove(0, 0, BLACK);
        game.applyMove(0, 1, WHITE);
        game.applyMove(0, 2, WHITE);
        MoveResult result = game.applyMove(0, 3, BLACK);

        assert(result.captured.size() == 2);
        assert(game.getBoard().getCell(0, 1) == EMPTY);
        assert(game.getBoard().getCell(0, 2) == EMPTY);
        assert(game.getBoard().getCaptures(BLACK) == 2);
    }

    void testAllEightCaptureDirections() {
        Board board;
        const int dr[] = {-1, -1, -1, 0, 0, 1, 1, 1};
        const int dc[] = {-1, 0, 1, -1, 1, -1, 0, 1};
        for (int d = 0; d < 8; ++d) {
            board.setCell(9 + dr[d], 9 + dc[d], WHITE);
            board.setCell(9 + 2 * dr[d], 9 + 2 * dc[d], WHITE);
            board.setCell(9 + 3 * dr[d], 9 + 3 * dc[d], BLACK);
        }
        std::vector<Point> captured = Rules::checkCaptures(board, 9, 9, BLACK);
        assert(captured.size() == 16);
    }

    void testTripleIsNotCaptured() {
        Board board;
        board.setCell(5, 0, BLACK);
        board.setCell(5, 1, WHITE);
        board.setCell(5, 2, WHITE);
        board.setCell(5, 3, WHITE);

        std::vector<Point> captured = Rules::checkCaptures(board, 5, 4, BLACK);
        assert(captured.empty());
    }

    void testDoubleThreeAndCaptureException() {
        Board board;
        // Playing (9,9) creates open threes horizontally and vertically.
        board.setCell(9, 8, BLACK);
        board.setCell(9, 10, BLACK);
        board.setCell(8, 9, BLACK);
        board.setCell(10, 9, BLACK);
        assert(Rules::isDoubleThree(board, 9, 9, BLACK));
        assert(!Rules::isLegalMove(board, 9, 9, BLACK));

        // A capture makes the same double-three legal, as required by the
        // official subject.
        board.setCell(8, 8, WHITE);
        board.setCell(7, 7, WHITE);
        board.setCell(6, 6, BLACK);
        assert(!Rules::checkCaptures(board, 9, 9, BLACK).empty());
        assert(Rules::isLegalMove(board, 9, 9, BLACK));
    }

    void testEngineRejectsForbiddenMove() {
        GameEngine game;
        game.applyMove(9, 8, BLACK);
        game.applyMove(9, 10, BLACK);
        game.applyMove(8, 9, BLACK);
        game.applyMove(10, 9, BLACK);

        bool rejected = false;
        try {
            game.applyMove(9, 9, BLACK);
        } catch (const std::invalid_argument&) {
            rejected = true;
        }
        assert(rejected);
    }

    void testCaptureCounterWin() {
        Board board;
        board.addCaptures(BLACK, 10);
        board.addCaptures(WHITE, 8);
        assert(Rules::hasTenCaptures(board, BLACK));
        assert(!Rules::hasTenCaptures(board, WHITE));
    }

    void testBreakableFive() {
        Board board;
        for (int c = 5; c <= 9; ++c)
            board.setCell(9, c, BLACK);

        // The extra black stone makes the end of the line capturable from a
        // perpendicular continuation: W at (7,5), BB at (8,5),(9,5), then
        // W can play (10,5) and remove the line's endpoint.
        board.setCell(8, 5, BLACK);
        board.setCell(7, 5, WHITE);
        assert(Rules::isFiveBreakable(board, BLACK));

        Board solidLine;
        for (int c = 5; c <= 9; ++c)
            solidLine.setCell(9, c, BLACK);
        assert(!Rules::isFiveBreakable(solidLine, BLACK));
    }

    void testBoardBoundsAndFullness() {
        Board board;
        assert(board.emptyCount() == BOARD_SIZE * BOARD_SIZE);
        assert(!board.isFull());
        for (int r = 0; r < BOARD_SIZE; ++r)
            for (int c = 0; c < BOARD_SIZE; ++c)
                board.setCell(r, c, ((r + c) % 2 == 0) ? BLACK : WHITE);
        assert(board.emptyCount() == 0);
        assert(board.isFull());
        assert(board.getCaptures(EMPTY) == 0);
    }

    void testAiReturnsALegalMove() {
        GameEngine game;
        game.applyMove(9, 9, BLACK);
        AI ai;
        ai.setTimeLimit(20);
        Point move = ai.getBestMove(game, WHITE);
        assert(Rules::isLegalMove(game.getBoard(), move.row, move.col, WHITE));
    }
}

int main() {
    Zobrist::init();
    testPairCapture();
    testAllEightCaptureDirections();
    testTripleIsNotCaptured();
    testDoubleThreeAndCaptureException();
    testEngineRejectsForbiddenMove();
    testCaptureCounterWin();
    testBreakableFive();
    testBoardBoundsAndFullness();
    testAiReturnsALegalMove();
    std::cout << "All Pente rule tests passed.\n";
    return 0;
}