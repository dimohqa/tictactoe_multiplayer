#ifndef TICTACTOE_STATUS_H
#define TICTACTOE_STATUS_H

#include <unistd.h>

enum StatusCode {
    MOVE = 1,
    STATE,
    WIN,
    LOSS,
    WRONG,
    PLAYER_JOINED,
    SECOND_PLAYER_JOINED,
    CREATED,
    GAME_EXIST,
    INVALID_COMMAND,
    SWITCH_TO_COMMAND,
    CONNECTED,
    MAIN_SERVER,
    DELETE_GAME,
};


bool sendStatus(int socket, StatusCode statusCode, bool isMainServer);

bool receiveStatus(int socket, StatusCode* statusCode);


#endif
