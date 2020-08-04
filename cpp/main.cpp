#include <iostream>

#include "2048.hpp"

using namespace Game;

int main() {
    setSeed(0);
    initTables();

    // Places two random tiles
    Board board = 0;
    addRandomTile(board);
    addRandomTile(board);

    char move;
    while (board) {
        printBoard(board);

        std::cin >> move;
        switch (move) {
            case 'w':
                board = slide(board, 0);
                break;
            case 'a':
                board = slide(board, 1);
                break;
            case 's':
                board = slide(board, 2);
                break;
            case 'd':
                board = slide(board, 3);
                break;
        }

        auto [val, pos] = getWorstTile(board);
        std::cout << val << " " << pos << std::endl;
    }

    std::cout << "Game Over!" << std::endl;
}
