#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <list>
#include <algorithm>
#include <csignal>
#include <arpa/inet.h>
#include "Player.h"
#include "Status.h"
#include "Game.h"
#include "sockets.h"
#include "Server.h"
#include "CellType.h"

using namespace std;

void *handle_client(void *arg);

void *handle_2_server(void *arg);

bool sendStateGame(int client_socket, int board[MAX_ROWS][MAX_ROWS]);

int connect_to_main_server(char *port);

int connect_to_spare2_server();

void *initSpareServer(void *arg);

int server_socket = 0;
char *server_ports[3] = {"50001", "50002", "50003"};

list<int> serverSocketList;
list<Game *> gameList;
bool isMainServer = false;
char *port;

pthread_mutex_t games_lock = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char *argv[]) {
    signal(SIGPIPE, SIG_IGN);
    if (argc != 3) {
        cout << "Incorrect length argc" << endl;
        return 0;
    }

    port = argv[1];

    server_socket = init_server(argv[1]);

    int new_socket = 0;

    if (atoi(argv[2])) {
        isMainServer = true;
        while (true) {
            int socket = 0;
            if ((socket = accept(server_socket, nullptr, nullptr)) < 0) {
                perror("Failed to accept client");
                close(socket);
            }
            serverSocketList.push_back(socket);

            if (serverSocketList.size() == 2)
                break;
        }
    } else {
        if (strcmp(port, server_ports[1]) == 0) {
            pthread_t thread_id;
            pthread_create(&thread_id, nullptr, initSpareServer, (void *) nullptr);
        }
        pthread_t thread_id;
        pthread_create(&thread_id, nullptr, handle_2_server, (void *) server_ports[0]);
    }

    while (true) {
        if ((new_socket = accept(server_socket, nullptr, nullptr)) < 0) {
            perror("Failed to accept client");
            close(server_socket);
            exit(1);
        }

        cout << "New client connected, socket: " << new_socket << endl;

        pthread_t thread_id;
        pthread_create(&thread_id, nullptr, handle_client, (void *) &new_socket);
    }

    return 0;
}

void *handle_2_server(void *arg) {
    char *port = (char *) arg;
    int server_socket = connect_to_main_server(port);

    handle_client((void *) &server_socket);
}

void *initSpareServer(void *arg) {
    int new_socket = init_spare_server(server_ports[2]);
    while (true) {
        int socket = 0;
        if ((socket = accept(new_socket, nullptr, nullptr)) < 0) {
            perror("Failed to accept client");
            close(socket);
        }
        serverSocketList.push_back(socket);

        if (serverSocketList.size() == 1)
            break;
    }
}

void *handle_client(void *arg) {
    int client_socket = *(int *) arg;
    bool client_connected = true;
    int row = 0, col = 0;

    Player player(client_socket, "");

    while (client_connected) {
        StatusCode statusCode;
        client_connected = receiveStatus(player.getSocket(), &statusCode);

        if (!client_connected) {
            if (strcmp(port, server_ports[2]) == 0 && !isMainServer) {
                int spare_socket = connect_to_spare2_server();

                if (spare_socket == 0) {
                    client_connected = false;
                    continue;
                }

                player.setSocket(spare_socket);
                client_connected = true;
                continue;
            }
        }
        if (client_connected) {
            switch (statusCode) {
                case CONNECTED: {
                    int isNewGame = 0;
                    receiveInt(player.getSocket(), &isNewGame);

                    char temp = '\0';
                    char buffer[1024];
                    int i;

                    for (i = 0; i < 1023 && temp != '\n' && client_connected; ++i) {
                        read(client_socket, &temp, 1);
                        buffer[i] = temp;
                    }

                    temp = '\0';
                    buffer[i] = '\0';
                    buffer[i - 1] = '\0';

                    player.setName(buffer);

                    pthread_mutex_lock(&games_lock);

                    auto foundedGame = find_if(gameList.begin(), gameList.end(), [player](Game *game) {
                        return game->getPlayer1().getName() == player.name ||
                               game->getPlayer2().getName() == player.name;
                    });

                    if (foundedGame != gameList.end()) {
                        if ((*foundedGame)->getPlayer1().getName() == player.getName()) {
                            (*foundedGame)->setPlayer1(player);
                        } else if ((*foundedGame)->getPlayer2().getName() == player.getName()) {
                            (*foundedGame)->setPlayer2(player);
                        }
                    } else {
                        auto freeGame = find_if(gameList.begin(), gameList.end(), [](Game *game) {
                            return game->getPlayer2().getSocket() == -1;
                        });

                        if (freeGame != gameList.end()) {
                            (*freeGame)->setPlayer2(player);
                            sendStatus(player.getSocket(), PLAYER_JOINED, isMainServer);
                            sendStatus((*freeGame)->getPlayer1().getSocket(), SECOND_PLAYER_JOINED, isMainServer);
                        } else {
                            Game *game = new Game(player);
                            gameList.push_back(game);
                            sendStatus(player.getSocket(), CREATED, isMainServer);
                        }
                    }

                    pthread_mutex_unlock(&games_lock);

                    if (strcmp(port, server_ports[0]) == 0) {
                        for (auto &iter : serverSocketList) {
                            const char *playerName = player.getName().c_str();
                            sendStatus(iter, CONNECTED, isMainServer);
                            sendInt(iter, 1);
                            snprintf(buffer, 1024, "%s\n", playerName);
                            write(iter, buffer, strlen(buffer));
                        }
                    }

                    if (strcmp(port, server_ports[1]) == 0 && !isNewGame) {
                        for (auto &iter : serverSocketList) {
                            const char *playerName = player.getName().c_str();
                            sendStatus(iter, CONNECTED, isMainServer);
                            sendInt(iter, 1);
                            snprintf(buffer, 1024, "%s\n", playerName);
                            write(iter, buffer, strlen(buffer));
                        }
                    }

                    break;
                }
                case MOVE: {
                    pthread_mutex_lock(&games_lock);
                    auto currentGame = find_if(gameList.begin(), gameList.end(),
                                               [player](Game *game) { return game->hasPlayer(player); });
                    pthread_mutex_unlock(&games_lock);

                    int type;
                    receiveInt(player.getSocket(), &type);
                    receiveInt(player.getSocket(), &row);
                    receiveInt(player.getSocket(), &col);

                    cout << "Socket: " << player.getSocket() << ", Player: " << player.getName() <<
                         " receive row = {" << row << "} and col = {" << col << "}" << endl;

                    if (isMainServer) {
                        for (auto &iter : serverSocketList) {
                            sendStatus(iter, MOVE, isMainServer);
                            sendInt(iter, type);
                            sendInt(iter, row);
                            sendInt(iter, col);
                        }
                    }

                    if (!(*currentGame)->board.isBlank(row, col)) {
                        sendStatus(client_socket, INVALID_COMMAND, isMainServer);
                    }

                    (*currentGame)->board.move(row, col, type);

                    (*currentGame)->board.DrawBoard();

                    if (isMainServer) {
                        bool wonIsTrue = false;
                        if ((*currentGame)->getPlayer1().getName() == player.getName()) {
                            wonIsTrue = (*currentGame)->board.typeIsWon(CROSS);
                        } else {
                            wonIsTrue = (*currentGame)->board.typeIsWon(CIRCLE);
                        }

                        if (wonIsTrue) {
                            sendStatus(player.getSocket(), STATE, isMainServer);
                            sendStateGame(player.getSocket(), (*currentGame)->board.board);

                            sendStatus(player.getSocket(), WIN, isMainServer);

                            Player other_player = (*currentGame)->getOtherPlayer(player);

                            sendStatus(other_player.getSocket(), STATE, isMainServer);
                            sendStateGame(other_player.getSocket(), (*currentGame)->board.board);

                            sendStatus(other_player.getSocket(), LOSS, isMainServer);

                            pthread_mutex_lock(&games_lock);

                            gameList.remove_if([player](Game *game) {
                                return game->hasPlayer(player);
                            });

                            for (auto &iter: serverSocketList) {
                                sendStatus(iter, DELETE_GAME, isMainServer);
                            }

                            pthread_mutex_unlock(&games_lock);

                            break;
                        }

                        sendStatus(client_socket, STATE, isMainServer);
                        sendStateGame(client_socket, (*currentGame)->board.board);

                        Player otherPlayer = (*currentGame)->getOtherPlayer(player);

                        sendStatus(otherPlayer.getSocket(), MOVE, isMainServer);
                        sendInt(otherPlayer.getSocket(), row);
                        sendInt(otherPlayer.getSocket(), col);

                    }

                    break;
                }
                case DELETE_GAME: {
                    gameList.remove_if([player](Game *game) {
                        return game->hasPlayer(player);
                    });

                    break;
                }
                case MAIN_SERVER: {
                    isMainServer = true;
                    break;
                }
                default: {
                    client_connected = sendStatus(player.getSocket(), INVALID_COMMAND, isMainServer);
                }
            }
        }
    }
}

int connect_to_main_server(char *port) {
    struct addrinfo *aip;
    struct addrinfo hint{};
    int err = 0;
    int server_socket;

    memset(&hint, 0, sizeof(hint));
    hint.ai_family = AF_INET;
    hint.ai_socktype = SOCK_STREAM;

    if ((err = getaddrinfo("nitro", port, &hint, &aip)) != 0) {
        cout << "Failed to get server address info: " << gai_strerror(err) << endl;
        exit(1);
    }

    if ((server_socket = socket(aip->ai_family, aip->ai_socktype, 0)) < 0) {
        cout << "Failed to create socket" << endl;
        exit(1);
    }

    if (connect(server_socket, aip->ai_addr, aip->ai_addrlen) < 0) {
        cout << "Failed to connect to server" << endl;
        exit(1);
    }

    cout << "Connected to main server successfully" << endl;

    return server_socket;
}

int connect_to_spare2_server() {
    sockaddr_in sockAddr{};

    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0) {
        cout << "Error socket" << endl;
        return 0;
    }

    bzero((char *) &sockAddr, sizeof(sockAddr));
    sockAddr.sin_family = AF_INET;
    sockAddr.sin_port = htons(50003);
    sockAddr.sin_addr.s_addr =
            inet_addr("127.0.0.1");

    int connectionStatus = connect(clientSocket, (sockaddr *) &sockAddr, sizeof(sockAddr));
    if (connectionStatus < 0) {
        cout << "Error connecting to socket!" << endl;
        return 0;
    }

    return clientSocket;
}

bool sendStateGame(int client_socket, int board[MAX_ROWS][MAX_ROWS]) {
    for (int i = 0; i < MAX_ROWS; ++i) {
        for (int j = 0; j < MAX_ROWS; ++j) {
            sendInt(client_socket, board[i][j]);
        }
    }
}
