#include "Player.h"

void Player::setName(string playerName) {
    name = std::move(playerName);
}

string Player::getName() {
    return name;
}

int Player::getSocket() {
    return socket;
}
