#ifndef TICTACTOE_SOCKETS_H
#define TICTACTOE_SOCKETS_H

#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <cstring>
#include <climits>

using namespace std;

bool sendInt(int socket, int number);

bool receiveInt(int socket, int *number);

int init_server(char *port);

char* allocate_hostname();

int init_spare_server(char *port);

#endif
