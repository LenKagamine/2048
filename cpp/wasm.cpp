#include <emscripten/bind.h>

#include <iostream>
#include <vector>

#include "2048.hpp"

using namespace Game;

Board board;

void setup(int seed) {
    setSeed(seed);
    initTables();
    board = 0;
}

void setBoard(std::vector<int> array) {
    if (array.size() != 16) {
        std::cout << "Error size!" << std::endl;
        return;
    }

    std::array<int, 16> b;
    for (int i = 0; i < 16; i++) {
        b[i] = array[i];
    }

    board = createBoard(b);
}

// New tile of value at position
int getMove(int position, int value) {
    board = setTile(board, position, value);
    return getBestMove(board);
}

void reset(int seed) {
    setSeed(seed);
    board = 0;
}

EMSCRIPTEN_BINDINGS(game) {
    emscripten::function("setup", &setup);
    emscripten::function("setBoard", &setBoard);
    emscripten::function("getMove", &getMove);
    emscripten::function("reset", &reset);
    emscripten::register_vector<int>("VecInt");
}
