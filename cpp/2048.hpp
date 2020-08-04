#ifndef H_2048
#define H_2048

#include <array>
#include <cstdint>

namespace Game {
// Store the board state in a 64-bit integer
// Stored left-to-right, top-to-bottom in LSB to MSB
using Board = uint_fast64_t;
using Row = uint_fast16_t;

constexpr int ROW_LEN = 4;                     // # of tiles per row/column
constexpr int BOARD_LEN = ROW_LEN * ROW_LEN;   // # of tiles in board
constexpr int TILE_SIZE = 4;                   // # bits per tile
constexpr int ROW_SIZE = TILE_SIZE * ROW_LEN;  // # bits per row

// Utility methods
Board createBoard(std::array<int, 16> board);
void printBoard(Board board);
void setSeed(int seed);

void initTables();
void addRandomTile(Board& board);
Board setTile(Board board, int position, int tile);
float evaluate(Board board);
Board slide(Board board, int direction);
int getBestMove(Board& board);
std::pair<int, int> getWorstTile(Board& board);

}  // namespace Game

#endif  // H_2048