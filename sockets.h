#ifndef TICTACTOE_SOCKETS_H
#define TICTACTOE_SOCKETS_H

#include <unistd.h>

bool sendInt(int socket, int number);

bool receiveInt(int socket, int *number);

#endif
