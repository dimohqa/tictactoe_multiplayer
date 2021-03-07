#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <list>
#include <algorithm>
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

int server_socket = 0;
char *server_ports[3] = {"50001", "50002", "50003"};

list<int> serverSocketList;
list<Game *> gameList;
bool isMainServer = false;

pthread_mutex_t games_lock = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char *argv[]) {
    if (argc != 3) {
        cout << "Incorrect length argc" << endl;
        return 0;
    }

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
            cout << "New server connected" << endl;
            serverSocketList.push_back(socket);

            if (serverSocketList.size() == 2)
                break;
        }
    } else {
        pthread_t thread_id;
        pthread_create(&thread_id, nullptr, handle_2_server, (void *) server_ports[0]);
    }

    for (auto &iter: serverSocketList) {
        cout << iter << endl;
    }

    while (true) {
        if ((new_socket = accept(server_socket, nullptr, nullptr)) < 0) {
            perror("Failed to accept client");
            close(server_socket);
            exit(1);
        }

        cout << "New client connected" << endl;

        pthread_t thread_id;
        pthread_create(&thread_id, nullptr, handle_client, (void *) &new_socket);
    }

    return 0;
}

void *handle_2_server(void *arg) {
    char *port = (char *) arg;
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
    serverSocketList.push_back(server_socket);
}

void *handle_client(void *arg) {
    int client_socket = *(int *) arg;
    bool client_connected = true;

    int row = 0, col = 0;

    Player player(client_socket);

    while (client_connected) {
        StatusCode statusCode;
        client_connected = receiveStatus(player.getSocket(), &statusCode);

        if (client_connected) {
            switch (statusCode) {
                case CONNECTED: {
                    pthread_mutex_lock(&games_lock);

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

                    pthread_mutex_unlock(&games_lock);



                    break;
                }
                case MOVE: {
                    pthread_mutex_lock(&games_lock);
                    auto currentGame = find_if(gameList.begin(), gameList.end(),
                                               [player](Game *game) { return game->hasPlayer(player); });
                    pthread_mutex_unlock(&games_lock);

                    receiveInt(player.getSocket(), &row);
                    receiveInt(player.getSocket(), &col);

                    cout << "Socket: " << player.getSocket()
                         << " receive row = {" << row << "} and col = {" << col << "}" << endl;

                    if (!(*currentGame)->board.isBlank(row, col)) {
                        sendStatus(client_socket, INVALID_COMMAND, isMainServer);
                    }

                    if ((*currentGame)->getPlayer1().getSocket() == player.getSocket()) {
                        (*currentGame)->board.playerMakeMove(row, col);
                    } else {
                        (*currentGame)->board.otherMakeMove(row, col);
                    }

                    (*currentGame)->board.DrawBoard();

                    bool wonIsTrue = false;
                    if ((*currentGame)->getPlayer1().getSocket() == player.getSocket()) {
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

                        gameList.remove_if([player] (Game *game) {
                            return game->hasPlayer(player);
                        });

                        pthread_mutex_unlock(&games_lock);

                        break;
                    }

                    sendStatus(client_socket, STATE, isMainServer);
                    sendStateGame(client_socket, (*currentGame)->board.board);

                    Player otherPlayer = (*currentGame)->getOtherPlayer(player);

                    sendStatus(otherPlayer.getSocket(), MOVE, isMainServer);
                    sendInt(otherPlayer.getSocket(), row);
                    sendInt(otherPlayer.getSocket(), col);

                    break;
                }
                default: {
                    client_connected = sendStatus(player.getSocket(), INVALID_COMMAND, isMainServer);
                }
            }
        }
    }
}

bool sendStateGame(int client_socket, int board[MAX_ROWS][MAX_ROWS]) {
    for (int i = 0; i < MAX_ROWS; ++i) {
        for (int j = 0; j < MAX_ROWS; ++j) {
            sendInt(client_socket, board[i][j]);
        }
    }
}
