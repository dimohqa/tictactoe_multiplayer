#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <cstring>
#include <zconf.h>
#include <list>
#include <algorithm>
#include "Player.h"
#include "Status.h"
#include "Game.h"
#include "sockets.h"

#define PORT "50002"
#define MAX_CLIENTS 10
#define BUF_SIZE 1024

using namespace std;

int init_server();
char* allocate_hostname();
void* handle_client(void* arg);
bool processCommand(char buffer[], Player &player, bool &client_connected);
void joinToGame(Player &player, const string& game_name);
void getGamesList(Player player);

int server_socket = 0;

list<Game*> gameList;
pthread_mutex_t games_lock = PTHREAD_MUTEX_INITIALIZER;

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

    int row = 0, col = 0;

    char buffer[BUF_SIZE];
    char temp = '\0';
    int i = 0;

    Player player(client_socket);

   while (client_connected) {
       if (player.getMode() == COMMAND) {
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
               Status::sendStatus(player.getSocket(), INVALID_COMMAND);
           }
       } else if (player.getMode() == INGAME) {
           pthread_mutex_lock(&games_lock);
           auto currentGame = find_if(gameList.begin(), gameList.end(),
                                      [player] (Game *game) { return game->hasPlayer(player); });
           pthread_mutex_unlock(&games_lock);

           StatusCode statusCode;
           client_connected = Status::receiveStatus(player.getSocket(), &statusCode);

           if (client_connected) {
               switch (statusCode) {
                   case MOVE: {
                       receiveInt(player.getSocket(), &row);
                       receiveInt(player.getSocket(), &col);

                       cout << "Player " << player.getName() << ", socket: " << player.getSocket()
                            << "receive row = {" << row << "} and col = {" << col << "}" << endl;

                       Player otherPlayer = (*currentGame)->getOtherPlayer(player);

                       Status::sendStatus(otherPlayer.getSocket(), MOVE);
                       sendInt(otherPlayer.getSocket(), row);
                       sendInt(otherPlayer.getSocket(), col);

                       break;
                   }
                   default: {
                       client_connected = Status::sendStatus(player.getSocket(), INVALID_COMMAND);
                   }
               }
           }
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
    } else if (command == "join" && num == 2) {
        joinToGame(player, arg);
    } else if (command == "list" && num == 1) {
        getGamesList(player);
    } else {
        valid_command = false;
    }

    return valid_command;
}

void joinToGame(Player &player, const string& game_name) {
    pthread_mutex_lock(&games_lock);

    auto iter = find_if(gameList.begin(), gameList.end(),
                        [game_name] (Game *game) { return game->getName() == game_name; });

    if (iter != gameList.end()) {
        if ((*iter)->getPlayer2().getSocket() == -1) {
            (*iter)->setPlayer2(player);
            player.setMode(INGAME);
            Status::sendStatus(player.getSocket(), PLAYER_JOINED);
            Status::sendStatus((*iter)->getPlayer1().getSocket(), SECOND_PLAYER_JOINED);
        } else {
            Status::sendStatus(player.getSocket(), GAME_EXIST);
        }
    } else {
        Game *game = new Game(game_name, player);
        gameList.push_back(game);
        Status::sendStatus(player.getSocket(), CREATED);
        player.setMode(INGAME);
    }

    pthread_mutex_unlock(&games_lock);
}

void getGamesList(Player player) {
    int countOpenGames = 0;

    Status::sendStatus(player.getSocket(), GAMES_LIST);

    pthread_mutex_lock(&games_lock);
    for (auto & iter : gameList) {
        if (iter->getPlayer2().getSocket() == -1) {
            write(player.getSocket(), iter->getName().c_str(), iter->getName().length());
            write(player.getSocket(), "\n", 1);
            ++countOpenGames;
        }
    }
    pthread_mutex_unlock(&games_lock);

    if (countOpenGames == 0) {
        string noOpenGamesMessage = "No open games\n";
        write(player.getSocket(), noOpenGamesMessage.c_str(), noOpenGamesMessage.length());
    }

    write(player.getSocket(), "\0", 1);
}