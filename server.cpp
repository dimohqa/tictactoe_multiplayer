#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <cstring>
#include <zconf.h>
#include "Player.h"
#include "Status.h"
#include "Game.h"

#define PORT "50002"
#define MAX_CLIENTS 10
#define BUF_SIZE 1024

using namespace std;

int init_server();
char* allocate_hostname();
void* handle_client(void* arg);
bool processCommand(char buffer[], Player &player, bool &client_connected);

int server_socket = 0;

Game game;
Status st;

int main() {
    server_socket = init_server();

    int new_socket = 0;

    while (true) {
        if ((new_socket = accept(server_socket, nullptr, nullptr)) < 0) {
            perror("Failed to accept client");
            close(server_socket);
            exit(1);
        }

        cout << "New client connected" << endl;

        pthread_t thread_id;
        pthread_create(&thread_id, nullptr, handle_client, (void*)&new_socket);
    }

    return 0;
}

char* allocate_hostname() {
    char *host;
    long maxsize = 0;

    if ((maxsize = sysconf(_SC_HOST_NAME_MAX)) < 0) {
        maxsize = HOST_NAME_MAX;
    }

    if ((host = (char*)malloc((size_t)maxsize)) == nullptr) {
        perror("Can't allocate hostname");
        exit(1);
    }

    if (gethostname(host, (size_t)maxsize) < 0) {
        perror("Couldn't retrieve hostname");
        exit(1);
    }

    return host;
}

int init_server() {
    int orig_socket = 0;
    char *hostname;

    int err = 0;
    int reuse = 1;

    struct addrinfo *aip;
    struct addrinfo hint{};

    memset(&hint, 0, sizeof(hint));
    hint.ai_family = AF_INET;
    hint.ai_socktype = SOCK_STREAM;

    hostname = allocate_hostname();
    if ((err = getaddrinfo(hostname, PORT, &hint, &aip)) != 0) {
        cout << "Failed to get address info: " << gai_strerror(err);
        free(hostname);
        exit(1);
    }
    free(hostname);

    if ((orig_socket = socket(aip->ai_addr->sa_family, aip->ai_socktype, 0)) < 0) {
        perror("Failed to create socket");
        exit(1);
    }

    if (setsockopt(orig_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) < 0) {
        perror("Failed to setup socket for address reuse");
        exit(1);
    }

    if (bind(orig_socket, aip->ai_addr, aip->ai_addrlen) < 0) {
        perror("Failed bind socket");
        close(orig_socket);
        exit(1);
    }

    freeaddrinfo(aip);

    if (listen(orig_socket, MAX_CLIENTS) < 0) {
        perror("Failed to start listening");
        close(orig_socket);
        exit(1);
    }

    return orig_socket;
}

void* handle_client(void* arg) {
    int client_socket = *(int*)arg;
    bool client_connected = true;

    char buffer[BUF_SIZE];
    char temp = '\0';
    int i = 0;

    Player player(client_socket);

   while (client_connected) {
       for (i = 0; i < (BUF_SIZE - 1) && temp != '\n' && client_connected; ++i) {
            if (read(client_socket, &temp, 1) == 0) {
                client_connected = false;
            } else {
                buffer[i] = temp;
            }
       }

       temp = '\0';
       buffer[i] = '\0';
       buffer[i - 1] = '\0';
       cout << "Received command \"" << buffer << "\" from " << player.getName() << endl;
       buffer[i - 1] = '\n';

       if (!processCommand(buffer, player, client_connected)) {
           st.sendStatus(player.getSocket(), INVALID_COMMAND);
       }
   }
}

bool processCommand(char buffer[], Player &player, bool &client_connected) {
    char s_command[BUF_SIZE], s_arg[BUF_SIZE];
    string command, arg;
    bool valid_command = true;

    int num = sscanf(buffer, "%s %s", s_command, s_arg);

    command = s_command;
    arg = s_arg;

    if (command == "register" && num == 2) {
        player.setName(arg);
        cout << "Registered player name \"" << arg << "\"" << endl;
        if (game.getCreatedGamePlayer().getSocket() == -1) {
            game.setCreatedGamePlayer(player);
            st.sendStatus(player.getSocket(), CREATED);
        } else {
            game.setPlayer(player);
            st.sendStatus(player.getSocket(), PLAYER_JOINED);
            st.sendStatus(game.getCreatedGamePlayer().getSocket(), SECOND_PLAYER_JOINED);
        }

    }

    return valid_command;
}