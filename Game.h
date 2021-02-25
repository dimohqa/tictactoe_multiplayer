#ifndef TICTACTOE_GAME_H
#define TICTACTOE_GAME_H

#include "Player.h"
#include <utility>

class Game {
public:
    Game(): player_1(-1), player_2(-1) {}

    Player getPlayer1();

    Player getPlayer2();

    Player getOtherPlayer(Player &player);

    void setPlayer1(Player player);

    void setPlayer2(Player player);
private:
    Player player_1;
    Player player_2;
};


#endif
