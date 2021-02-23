#ifndef TICTACTOE_PLAYER_H
#define TICTACTOE_PLAYER_H

#include <iostream>
#include <string>
#include <utility>

using namespace std;


class Player {
public:
    Player(int socket): socket(socket), name("No_name") {}

    void setName(string name);

    string getName();

    int getSocket();
private:
    int socket = 0;
    string name;
};


#endif
