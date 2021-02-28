#ifndef TICTACTOE_STATUS_H
#define TICTACTOE_STATUS_H

#include <unistd.h>

enum StatusCode {
    MOVE,
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
};


class Status {
public:
    static bool sendStatus(int socket, StatusCode statusCode);

    static bool receiveStatus(int socket, StatusCode* statusCode);
};


#endif
