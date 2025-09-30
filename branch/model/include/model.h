#ifndef MODEL_H

typedef struct Board Board;

typedef struct Tile Tile;

Board *boardMake(int seed, int width, int height);

void boardFree(Board *brd);

int boardValueAt(Board *brd, int row, int col);

int boardConnectedAt(Board *brd, int row, int col);

void boardRotateAt(Board *brd, int row, int col);

#endif // !MODEL_H
