#ifndef TICTACTOE_SERVER_H
#define TICTACTOE_SERVER_H

#include <iostream>
#include <cstring>
#include <utility>

using namespace std;

class Server {
public:
    Server(string port)
            : port(std::move(port)), socket(0) {}

    string getPort();

    pthread_t* getThread();

    void setSocket(int sock);

    int getSocket();
private:
    string port;
    int socket;
    pthread_t thread_id{};
};

#endif
