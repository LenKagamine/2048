# Evil 2048

Play now at [2048.michael.kim](https://2048.michael.kim)!

A clone of 2048 where the game is fighting back! Tiles are placed in the most inconvenient places to hinder your progress towards 2048.

## Encoding the Board

The board state consists of 16 tiles containing only powers of two (a blank is represented as 0). The best 2048 solvers that I've been able to find cannot reach the 65,536 tile. By limiting the largest tile to be 32,768 = 2^15, each tile can be stored using 4 bits by storing its log_2. With 16 tiles, this brings the size of the board to snuggly into 64 bits.

<details>
<summary>Interesting note:</summary>
Since only 2 and 4 tiles can spawn, the theoretically largest tile possible is 131,072 = 2^17 using a board state such as:

```
    4     4     8   16
  256   128    64   32
  512  1024  2048 4096
65536 32768 16384 8192
```

However, the RNG must be favourable in order for this to occur as specific tiles are required to reach this state. For example, the last tile must be a 4. When the game is placing tiles adversarially, it will be impossible to reach this state.

</details>

## Autoplay

The AI solver is implemented with expectimax, assuming that the tile generation is random. When the AI assumes evil tiles (i.e. the worst tile generation), it performs marginally better.

## Evil Tiles

The evil tiles is implemented with minimax, assuming that the player plays as best as it can.

The solver and evil tiles both share the same heuristic
