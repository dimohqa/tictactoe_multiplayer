#ifndef TICTACTOE_PLAYER_H
#define TICTACTOE_PLAYER_H

#include <iostream>
#include <string>
#include <utility>

using namespace std;

class Player {
public:
    Player(int socket, string name): socket(socket), name(std::move(name)) {}

    int getSocket();

    string getName();

    void setName(string playerName);

    void setSocket(int newSocket);

    string name;
private:
    int socket = 0;
};


#endif
