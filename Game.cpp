#include "Game.h"

Player Game::getPlayer1() {
    return player_1;
}

Player Game::getPlayer2() {
    return player_2;
}

void Game::setPlayer1(Player player) {
    player_1 = std::move(player);
}

void Game::setPlayer2(Player player) {
    player_2 = std::move(player);
}

Player Game::getOtherPlayer(Player &player) {
    Player other(-1);

    if (player.getSocket() == player_1.getSocket()) {
        other = player_2;
    } else if (player.getSocket() == player_2.getSocket()) {
        other = player_1;
    }

    return other;
}

bool Game::hasPlayer(Player player) {
    if (player_1.getSocket() == player.getSocket() || player_2.getSocket() == player.getSocket()) {
        return true;
    }

    return false;
}
