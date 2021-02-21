#ifndef TICTACTOE_BOARD_H
#define TICTACTOE_BOARD_H

#include <iostream>

using namespace std;

#define MAX_ROWS 3

enum CellType {
    CROSS,
    CIRCLE,
    BLANK
};

class Board {
    Board();

    void playerMakeMove(int row, int col);

    void otherMakeMove(int row, int col);

    void setType(CellType typeCell);

    bool isBlank(int row, int col);

    char printCell(CellType typeCell);

    void DrawBoard();

    bool typeIsWon(CellType type);

    bool isWon();

    CellType board[MAX_ROWS][MAX_ROWS]{};
    CellType type;
};


#endif
