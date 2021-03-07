#ifndef TICTACTOE_PLAYER_H
#define TICTACTOE_PLAYER_H

#include <iostream>
#include <string>
#include <utility>

using namespace std;

class Player {
public:
    Player(int socket): socket(socket) {}

    int getSocket();
private:
    int socket = 0;
};


#endif
