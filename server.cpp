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

#define BUF_SIZE 1024

using namespace std;

void* handle_client(void* arg);
void* handle_2_server(void* arg);
bool processCommand(char buffer[], Player &player, bool &client_connected);
void joinToGame(Player &player, const string& game_name);
void getGamesList(Player player);
bool sendStateGame(int client_socket, int board[MAX_ROWS][MAX_ROWS]);

int server_socket = 0;
char* server_ports[3] = {"50001", "50002", "50003"};
list<int> serverSocketList;

list<Game*> gameList;
pthread_mutex_t games_lock = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cout << "Incorrect length argc" << endl;
        return 0;
    }

    server_socket = init_server(argv[1]);

    int new_socket = 0;

    if (atoi(argv[2])) {
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

    for (auto & iter: serverSocketList) {
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
        pthread_create(&thread_id, nullptr, handle_client, (void*)&new_socket);
    }

    return 0;
}

void* handle_2_server(void* arg) {
    char* port = (char*)arg;
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
               sendStatus(player.getSocket(), INVALID_COMMAND);
           }
       } else if (player.getMode() == INGAME) {
           pthread_mutex_lock(&games_lock);
           auto currentGame = find_if(gameList.begin(), gameList.end(),
                                      [player] (Game *game) { return game->hasPlayer(player); });
           pthread_mutex_unlock(&games_lock);

           StatusCode statusCode;
           client_connected = receiveStatus(player.getSocket(), &statusCode);

           if (client_connected) {
               switch (statusCode) {
                   case MOVE: {
                       receiveInt(player.getSocket(), &row);
                       receiveInt(player.getSocket(), &col);

                       cout << "Player " << player.getName() << ", socket: " << player.getSocket()
                            << " receive row = {" << row << "} and col = {" << col << "}" << endl;

                       if (!(*currentGame)->board.isBlank(row, col)) {
                           sendStatus(client_socket, INVALID_COMMAND);
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
                           sendStatus(player.getSocket(), STATE);
                           sendStateGame(player.getSocket(), (*currentGame)->board.board);

                           if (sendStatus(player.getSocket(), WIN)) {
                               player.setMode(COMMAND);
                           }

                           Player other_player = (*currentGame)->getOtherPlayer(player);

                           sendStatus(other_player.getSocket(), STATE);
                           sendStateGame(other_player.getSocket(), (*currentGame)->board.board);

                           sendStatus(other_player.getSocket(), LOSS);

                           (*currentGame)->board.resetBoard();

                           break;
                       }

                       sendStatus(client_socket, STATE);
                       sendStateGame(client_socket, (*currentGame)->board.board);

                       Player otherPlayer = (*currentGame)->getOtherPlayer(player);

                       sendStatus(otherPlayer.getSocket(), MOVE);
                       sendInt(otherPlayer.getSocket(), row);
                       sendInt(otherPlayer.getSocket(), col);

                       break;
                   }
                   case SWITCH_TO_COMMAND: {
                       player.setMode(COMMAND);
                       break;
                   }
                   default: {
                       client_connected = sendStatus(player.getSocket(), INVALID_COMMAND);
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
        sendStatus(player.getSocket(), REGISTERED);
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
            sendStatus(player.getSocket(), PLAYER_JOINED);
            sendStatus((*iter)->getPlayer1().getSocket(), SECOND_PLAYER_JOINED);
        } else {
            sendStatus(player.getSocket(), GAME_EXIST);
        }
    } else {
        Game *game = new Game(game_name, player);
        gameList.push_back(game);
        sendStatus(player.getSocket(), CREATED);
        player.setMode(INGAME);
    }

    pthread_mutex_unlock(&games_lock);
}

void getGamesList(Player player) {
    int countOpenGames = 0;

    sendStatus(player.getSocket(), GAMES_LIST);

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

bool sendStateGame(int client_socket, int board[MAX_ROWS][MAX_ROWS]) {
    for (int i = 0; i < MAX_ROWS; ++i) {
        for (int j = 0; j < MAX_ROWS; ++j) {
            sendInt(client_socket, board[i][j]);
        }
    }
}
