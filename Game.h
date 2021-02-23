#ifndef TICTACTOE_GAME_H
#define TICTACTOE_GAME_H

#include "Player.h"
#include <utility>

class Game {
public:
    Game(): player_1(-1), player_2(-1) {}

    Player getCreatedGamePlayer();

    Player getPlayer();

    void setCreatedGamePlayer(Player player);
    void setPlayer(Player player);
private:
    Player player_1;
    Player player_2;
};


#endif
