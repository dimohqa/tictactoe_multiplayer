#include "Server.h"

string Server::getPort() {
    return port;
}

pthread_t* Server::getThread() {
    return &thread_id;
}

void Server::setSocket(int sock) {
    socket = sock;
}

int Server::getSocket() {
    return socket;
}
