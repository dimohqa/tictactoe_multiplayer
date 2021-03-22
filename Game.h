#ifndef TICTACTOE_GAME_H
#define TICTACTOE_GAME_H

#include <utility>
#include "Player.h"
#include "Board.h"

class Game {
public:
    /*Game(): player_1(-1), player_2(-1), board(* new Board) {}*/

    Game(Player player): player_1(std::move(player)), player_2(-1, ""), board(* new Board) {}

    Player getPlayer1();

    Player getPlayer2();

    Player getOtherPlayer(Player &player);

    bool hasPlayer(Player player);

    void setPlayer1(Player player);

    void setPlayer2(Player player);

    Board &board;
private:
    Player player_1;
    Player player_2;
};


#endif
