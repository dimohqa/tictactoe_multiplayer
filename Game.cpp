#include "Game.h"

Player Game::getCreatedGamePlayer() {
    return player_1;
}

Player Game::getPlayer() {
    return player_2;
}

void Game::setCreatedGamePlayer(Player player) {
    player_1 = std::move(player);
}

void Game::setPlayer(Player player) {
    player_2 = std::move(player);
}
