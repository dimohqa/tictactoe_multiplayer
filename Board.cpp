#include "Board.h"

Board::Board(): type(CROSS) {
    for (int i = 0; i < MAX_ROWS; ++i) {
        for (int j = 0; j < MAX_ROWS; ++j) {
            board[i][j] = BLANK;
        }
    }
}

void Board::setType(CellType typeCell) {
    type = typeCell;
}

bool Board::isBlank(int row, int col) {
    if (board[row][col] == BLANK) {
        return true;
    }

    return false;
}

char Board::printCell(CellType typeCell) {
    char cell = ' ';
    if (typeCell == CROSS) {
        cell = 'X';
    }
    if (typeCell == CIRCLE) {
        cell = 'O';
    }

    return cell;
}

void Board::DrawBoard() {
    cout << printCell(board[0][0]) << "|"
         << printCell(board[0][1]) << "|"
         << printCell(board[0][2]) << endl;

    cout << "-----" << endl;

    cout << printCell(board[1][0]) << "|"
         << printCell(board[1][1]) << "|"
         << printCell(board[1][2]) << endl;

    cout << "-----" << endl;

    cout << printCell(board[2][0]) << "|"
         << printCell(board[2][1]) << "|"
         << printCell(board[2][2]) << endl;
}

void Board::playerMakeMove(int row, int col) {
    board[row][col] = type;
}

void Board::otherMakeMove(int row, int col) {
    if (type == CROSS) {
        board[row][col] = CIRCLE;
    } else {
        board[row][col] = CROSS;
    }
}

bool Board::typeIsWon(CellType type) {
    bool isWon = false;

    int wonCount = MAX_ROWS;

    int count = 0;

    for (int i = 0; i < MAX_ROWS; ++i) {
        for (int j = 0; j < MAX_ROWS; ++j) {
            if (board[i][j] == type) {
                count++;
            }
        }

        if (count == wonCount) {
            return true;
        }

        count = 0;
    }

    for (int i = 0; i < MAX_ROWS; ++i) {
        for (int j = 0; j < MAX_ROWS; ++j) {
            if (board[j][i] == type) {
                count++;
            }
        }

        if (count == wonCount) {
            return true;
        }

        count = 0;
    }

    for (int i = 0; i < MAX_ROWS; ++i) {
        if (board[i][i] == type) {
            count++;
        }

        if (count == wonCount) {
            return true;
        }
    }

    count = 0;

    for (int i = 0, j = MAX_ROWS - 1; i < MAX_ROWS; ++i, --j) {
        if (board[i][j] == type) {
            count++;
        }

        if (count == wonCount) {
            return true;
        }
    }

    return false;
}

bool Board::isWon() {
    return typeIsWon(type);
}

int Board::getSize() {
    return MAX_ROWS;
}


