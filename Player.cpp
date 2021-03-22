#include "Player.h"

int Player::getSocket() {
    return socket;
}

string Player::getName() {
    return name;
}

void Player::setName(string playerName) {
    name = playerName;
}

void Player::setSocket(int newSocket) {
    socket = newSocket;
}
