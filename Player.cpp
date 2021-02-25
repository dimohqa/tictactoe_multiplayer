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

Mode Player::getMode() {
    return mode;
}

void Player::setMode(Mode modePlayer) {
    mode = modePlayer;
}
