#ifndef MODEL_H
#define MODEL_H

enum TileType { NONE, T, I, L, E };

typedef struct Board Board;

typedef struct Tile Tile;

Board *boardMake(int seed, int width, int height);

void boardFree(Board *brd);

int boardValueAt(Board *brd, int row, int col);

int boardTileDegreeAt(Board *brd, int row, int col);

enum TileType boardTileTypeAt(Board *brd, int row, int col);

void boardPrint(Board *brd);

int boardConnectedAt(Board *brd, int row, int col);

void boardRotateAt(Board *brd, int row, int col);

#endif // !MODEL_H
