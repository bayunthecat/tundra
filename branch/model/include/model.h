#ifndef MODEL_H

typedef struct Board Board;

typedef struct Tile Tile;

Board *makeBoard(int seed, int width, int height);

void freeBoard(Board *brd);

int boardValueAt(Board *brd, int row, int col);

#endif // !MODEL_H
