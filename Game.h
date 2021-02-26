#ifndef TICTACTOE_GAME_H
#define TICTACTOE_GAME_H

#include "Player.h"
#include <utility>

class Game {
public:
    Game(string name): player_1(-1), player_2(-1), name(std::move(name)) {}

    Game(string name, Player player): player_1(std::move(player)), player_2(-1), name(std::move(name)) {}

    Player getPlayer1();

    Player getPlayer2();

    Player getOtherPlayer(Player &player);

    bool hasPlayer(Player player);

    void setPlayer1(Player player);

    void setPlayer2(Player player);

    string getName();
private:
    Player player_1;
    Player player_2;
    string name;
};


#endif
