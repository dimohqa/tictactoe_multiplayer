#ifndef TICTACTOE_BOARD_H
#define TICTACTOE_BOARD_H

#include <iostream>

using namespace std;

#define MAX_ROWS 3

class Board {
public:
    Board();

    void playerMakeMove(int row, int col);

    void otherMakeMove(int row, int col);

    void setType(int typeCell);

    bool isBlank(int row, int col);

    char printCell(int typeCell);

    void DrawBoard();

    bool typeIsWon(int type);

    bool isWon();

    int getSize();

    void resetBoard();

    int board[MAX_ROWS][MAX_ROWS]{};
    int type;
};


#endif
