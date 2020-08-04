#include "2048.hpp"

#include <algorithm>
#include <bit>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <random>

namespace Game {

// For placing random tiles
std::random_device rd;
std::mt19937 engine(rd());
std::uniform_int_distribution<> dist(0, 15);
std::uniform_int_distribution<> distTile(0, 9);

// Lookup tables
// 4 tiles per row, 16 possible tiles -> 16^4 = 65536 possible rows
std::array<Row, 65536> leftSlides{};
std::array<Row, 65536> rightSlides{};
std::array<Board, 65536> upSlides{};
std::array<Board, 65536> downSlides{};
std::array<float, 65536> heuristic{};

// Infinity for expectimax
constexpr int INF = 1E8;

// Compress board state into 64-bit int
Board createBoard(std::array<int, BOARD_LEN> board) {
    Board b = 0;
    for (auto it = board.rbegin(); it != board.rend(); it++) {
        unsigned digit = *it;
        if (digit == 0) {
            b <<= TILE_SIZE;
        } else {
            b = b << TILE_SIZE | std::countr_zero(digit);
        }
    }
    return b;
}

// Pretty print board
void printRow(Row row) {
    for (int x = 0; x < ROW_LEN; x++) {
        int tile = row & 0xF;
        row >>= ROW_LEN;
        std::cout << std::setw(6);
        if (tile == 0) {
            std::cout << 0;
        } else {
            std::cout << (1 << tile);
        }
    }
    std::cout << std::endl;
}
void printBoard(Board board) {
    for (int y = 0; y < ROW_LEN; y++) {
        Row row = board & 0xFFFF;
        printRow(row);
        board >>= ROW_SIZE;
    }
    std::cout << std::endl;
}

void setSeed(int seed) { engine.seed(seed); }

inline Board transpose(Board board) {
    Board next = 0;
    next |= board & 0xF0000F0000F0000F;          // 0 5 A F
    next |= (board & 0x0000F0000F0000F0) << 12;  // 1 6 B
    next |= (board & 0x00000000F0000F00) << 24;  // 2 7
    next |= (board & 0x000000000000F000) << 36;  // 3
    next |= (board & 0x0F0000F0000F0000) >> 12;  // 4 9 E
    next |= (board & 0x00F0000F00000000) >> 24;  //   8 D
    next |= (board & 0x000F000000000000) >> 36;  //     C
    return next;
}

float calculateHeuristic(const std::array<int, ROW_LEN>& row, Row s) {
    // Openness has twice the effect due to summing row-wise and column-wise
    int openness = 0;  // # of blank tiles
    for (int i = 0; i < ROW_LEN; i++) {
        if (row[i] == 0) {
            openness++;
        }
    }

    int smoothness = 0;  // # of matching adjacent tiles (ignoring blanks)
    for (int i = 0; i < ROW_LEN; i++) {
        if (row[i] != 0) {
            int temp = row[i];

            int i2 = i + 1;
            while (i2 < ROW_LEN && row[i2] == 0) i2++;
            if (i2 < ROW_LEN) {
                smoothness -= std::abs(temp - row[i2]);
            }
        }
    }

    // Monotonicity
    int leftMon = 0;
    int rightMon = 0;
    for (int i = 0; i < ROW_LEN - 1; i++) {
        if (row[i] > row[i + 1]) {
            leftMon += row[i + 1] - row[i];
        } else {
            rightMon += row[i] - row[i + 1];
        }
    }
    int monotonicity = std::max(leftMon, rightMon);

    return openness + smoothness + monotonicity;
}

// Init
void initTables() {
    for (int s = 0; s < 65536; s++) {
        // Decompress row
        std::array<int, ROW_LEN> row = {
            s & 0xF,
            (s >> 4) & 0xF,
            (s >> 8) & 0xF,
            (s >> 12) & 0xF,
        };
        // Reverse row (for shifting right and down)
        Row reverse = row[3] | (row[2] << 4) | (row[1] << 8) | (row[0] << 12);

        // Calculate heuristic
        heuristic[s] = calculateHeuristic(row, s);

        if (s > 0) {
            // Shift towards LSB ("left")
            for (int i = 0; i < 3; i++) {
                // Find next non-zero tile
                int j = i + 1;
                while (j < 4 && row[j] == 0) j++;
                if (j < 4) {
                    if (row[i] == 0) {
                        // Current tile is blank, move tile over and try again
                        row[i] = row[j];
                        row[j] = 0;
                        i--;
                    } else if (row[i] == row[j]) {
                        // Found matching pair of tiles, combine
                        if (row[i] != 0xF) {
                            // Max tile = 32,768 = 2^15 -> 0xF
                            row[i]++;
                        }
                        row[j] = 0;
                    }
                }
            }
        }

        // Compress row
        leftSlides[s] = row[0] | (row[1] << 4) | (row[2] << 8) | (row[3] << 12);
        rightSlides[reverse] =
            row[3] | (row[2] << 4) | (row[1] << 8) | (row[0] << 12);

        upSlides[s] = row[0] | ((Board)row[1] << 16) | ((Board)row[2] << 32) |
                      ((Board)row[3] << 48);
        downSlides[reverse] = row[3] | ((Board)row[2] << 16) |
                              ((Board)row[1] << 32) | ((Board)row[0] << 48);
    }
}

inline Board slideLeft(Board board) {
    return (Board)leftSlides[board & 0xFFFF] |
           (Board)leftSlides[(board >> 16) & 0xFFFF] << 16 |
           (Board)leftSlides[(board >> 32) & 0xFFFF] << 32 |
           (Board)leftSlides[(board >> 48) & 0xFFFF] << 48;
}

inline Board slideRight(Board board) {
    return (Board)rightSlides[board & 0xFFFF] |
           (Board)rightSlides[(board >> 16) & 0xFFFF] << 16 |
           (Board)rightSlides[(board >> 32) & 0xFFFF] << 32 |
           (Board)rightSlides[(board >> 48) & 0xFFFF] << 48;
}

inline Board slideUp(Board board) {
    Board t = transpose(board);
    return upSlides[t & 0xFFFF] | upSlides[(t >> 16) & 0xFFFF] << 4 |
           upSlides[(t >> 32) & 0xFFFF] << 8 |
           upSlides[(t >> 48) & 0xFFFF] << 12;
}

inline Board slideDown(Board board) {
    Board t = transpose(board);
    return downSlides[t & 0xFFFF] | downSlides[(t >> 16) & 0xFFFF] << 4 |
           downSlides[(t >> 32) & 0xFFFF] << 8 |
           downSlides[(t >> 48) & 0xFFFF] << 12;
}

Board setTile(Board board, int position, int tile) {
    // Position: 0 1 2 3 / 4 5 6 7 / 8 9 a b / c d e f
    // Assume tile at position is blank (0)
    // Tile is log of value
    board |= (Board)(tile) << (4 * position);
    return board;
}

inline int getTile(Board board, int position) {
    return (board >> (4 * position)) & 0xF;
}

void addRandomTile(Board& board) {
    auto tile = distTile(engine) == 0 ? 2 : 1;
    while (true) {
        auto position = dist(engine);
        if (getTile(board, position) == 0) {
            board = setTile(board, position, tile);
            return;
        }
    }
}

float evaluate(Board board) {
    // Sum heuristics row-wise
    float rows = heuristic[board & 0xFFFF] + heuristic[(board >> 16) & 0xFFFF] +
                 heuristic[(board >> 32) & 0xFFFF] +
                 heuristic[(board >> 48) & 0xFFFF];

    // Sum heuristics column-wise
    Board t = transpose(board);
    float cols = heuristic[t & 0xFFFF] + heuristic[(t >> 16) & 0xFFFF] +
                 heuristic[(t >> 32) & 0xFFFF] + heuristic[(t >> 48) & 0xFFFF];
    return rows + cols;
}

Board slide(Board board, int direction) {
    if (direction == 0) return slideUp(board);
    if (direction == 1) return slideLeft(board);
    if (direction == 2) return slideDown(board);
    return slideRight(board);
}

// Best move assuming random tile generation
float expectimax(Board board, int depth = 3, bool isPlayer = false) {
    if (depth == 0) {
        // Leaf node
        return evaluate(board);
    }

    if (isPlayer) {
        // Maximize
        float alpha = -INF;

        // Moves
        for (int dir = 0; dir < 4; dir++) {
            Board next = slide(board, dir);
            if (next != board) {
                alpha = std::max(alpha, expectimax(next, depth - 1, false));
            }
        }

        return alpha;
    }

    // Chance
    float alpha = 0;
    int numEmpty = 0;
    Board temp = board;

    for (int i = 0; i < BOARD_LEN; i++) {
        int tile = temp & 0xF;
        if (tile == 0) {
            numEmpty++;
            // Place 2
            Board b2 = setTile(board, i, 1);
            alpha += 0.9f * expectimax(b2, depth, true);

            // Place 4
            Board b4 = setTile(board, i, 2);
            alpha += 0.1f * expectimax(b4, depth, true);
        }
        temp >>= 4;
    }

    return alpha / numEmpty;
}

// Returns direction
int getBestMove(Board& board) {
    // Maximize
    int bestMove = 0;
    Board bestBoard = 0;
    float alpha = -INF;

    // Moves
    for (int dir = 0; dir < 4; dir++) {
        Board next = slide(board, dir);
        if (next != board) {
            float result = expectimax(next);
            if (result >= alpha) {
                alpha = result;
                bestMove = dir;
                bestBoard = next;
            }
        }
    }

    board = bestBoard;

    return bestMove;
}

// Worst tile generation assuming best movement
float minimax(Board board, int depth = 3, float alpha = -INF, float beta = INF,
              bool isPlayer = true) {
    if (depth == 0) {
        // Leaf node
        return evaluate(board);
    }

    if (isPlayer) {
        // Maximize
        float result = -INF;

        // Moves
        for (int dir = 0; dir < 4; dir++) {
            Board next = slide(board, dir);
            if (next != board) {
                result = std::max(result,
                                  minimax(next, depth - 1, alpha, beta, false));
                alpha = std::max(alpha, result);

                if (alpha > beta) {
                    break;
                }
            }
        }

        return result;
    }

    // Chance
    float result = INF;
    Board temp = board;

    for (int i = 0; i < BOARD_LEN; i++) {
        int tile = temp & 0xF;
        if (tile == 0) {
            // Place 2
            Board b2 = setTile(board, i, 1);
            float r2 = minimax(b2, depth, alpha, beta, true);
            result = std::min(result, r2);

            // Place 4
            Board b4 = setTile(board, i, 2);
            float r4 = minimax(b4, depth, alpha, beta, true);
            result = std::min(result, r4);

            beta = std::min(beta, result);
            if (beta <= alpha) {
                break;
            }
        }
        temp >>= 4;
    }

    return result;
}

// Returns direction
std::pair<int, int> getWorstTile(Board& board) {
    // Maximize
    int worstTile = 0;
    int worstPos = 0;
    Board worstBoard = 0;
    float result = INF;

    Board temp = board;
    for (int i = 0; i < BOARD_LEN; i++) {
        int tile = temp & 0xF;
        if (tile == 0) {
            // Place 2
            Board b2 = setTile(board, i, 1);
            float r2 = minimax(b2);
            if (r2 < result) {
                result = r2;
                worstTile = 1;
                worstPos = i;
                worstBoard = b2;
            }

            // Place 4
            Board b4 = setTile(board, i, 2);
            float r4 = minimax(b4);
            if (r4 < result) {
                result = r4;
                worstTile = 2;
                worstPos = i;
                worstBoard = b4;
            }
        }
        temp >>= 4;
    }

    board = worstBoard;
    return {worstTile, worstPos};
}

}  // namespace Game