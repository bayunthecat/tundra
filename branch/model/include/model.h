#ifndef MODEL_H

typedef struct Board Board;

typedef struct Tile Tile;

Board *makeBoard(int width, int height);

void freeBoard(Board *brd);

#endif // !MODEL_H
