#ifndef TICTACTOE_STATUS_H
#define TICTACTOE_STATUS_H

#include <unistd.h>

enum StatusCode {
    PLAYER_JOINED,
    SECOND_PLAYER_JOINED,
    CREATED,
    MOVE,
    PLAYER_DISCONNECT,
    SERVER_ERROR,
    GAME_EXIST,
    WIN,
    LOSS,
    TURN,
    INVALID_COMMAND,
};


class Status {
public:
    static bool sendStatus(int socket, StatusCode statusCode);

    static bool receiveStatus(int socket, StatusCode* statusCode);
};


#endif
