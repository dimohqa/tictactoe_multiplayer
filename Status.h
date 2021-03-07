#ifndef TICTACTOE_STATUS_H
#define TICTACTOE_STATUS_H

#include <unistd.h>

enum StatusCode {
    MOVE,
    STATE,
    WIN,
    LOSS,
    WRONG,
    PLAYER_JOINED,
    SECOND_PLAYER_JOINED,
    CREATED,
    REGISTERED,
    GAMES_LIST,
    GAME_EXIST,
    INVALID_COMMAND,
    SWITCH_TO_COMMAND,
    CONNECTED,
};


bool sendStatus(int socket, StatusCode statusCode);

bool receiveStatus(int socket, StatusCode* statusCode);


#endif
