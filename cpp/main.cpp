#include <iostream>

#include "2048.hpp"

using namespace Game;

int main() {
    setSeed(0);
    initTables();

    // Places two random tiles
    Board board = 0;
    addRandomTile(board);

    while (board) {
        addRandomTile(board);
        printBoard(board);
        getBestMove(board);
    }

    std::cout << "Game Over!" << std::endl;
}
