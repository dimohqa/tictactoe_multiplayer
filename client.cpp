#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netdb.h>
#include <cstring>
#include <climits>
#include "Board.h"
#include "Status.h"
#include "sockets.h"

#define PORT "50002"
#define BUF_SIZE 1024

using namespace std;

int init_client(char* server_name);
void serverDisconnected();
bool makeMove(Board &board);

enum Menu { LIST, NEW_GAME, JOIN_GAME };
int client_socket = 0;

int main(int argc, char* argv[]) {
    char buffer[1024];
    int row, col;
    bool game_over = false;
    bool ingame = false;
    char temp;

    StatusCode statusCode;
    Board board;

    if (argc != 3) {
        cout << "error, enter server address" << endl;
        exit(1);
    }

    client_socket = init_client(argv[1]);

    snprintf(buffer, BUF_SIZE, "register %s\n", argv[2]);
    write(client_socket, buffer, strlen(buffer));

    while (!game_over) {
        if (!ingame) {
            cout << "Choose command: ";

            fgets(buffer, BUF_SIZE - 1, stdin);

            int i;

            sscanf(buffer, "%d", &i);

            char buf[BUF_SIZE];

            switch (i) {
                case LIST:
                    snprintf(buffer, BUF_SIZE, "list\n");
                    write(client_socket, buffer, strlen(buffer));
                    break;
                case NEW_GAME:
                    cout << "Enter new game name: ";
                    fgets(buf, BUF_SIZE - 1, stdin);
                    snprintf(buffer, BUF_SIZE, "join %s\n", buf);
                    write(client_socket, buffer, strlen(buffer));
                    break;
                case JOIN_GAME:
                    cout << "Enter join game name: ";
                    fgets(buf, BUF_SIZE - 1, stdin);
                    snprintf(buffer, BUF_SIZE, "join %s\n", buf);
                    write(client_socket, buffer, strlen(buffer));
                default:
                    break;
            }
        }

        if (!Status::receiveStatus(client_socket, &statusCode)) {
            serverDisconnected();
        }

        switch (statusCode) {
            case CREATED: {
                cout << "Game created! Waiting your opponent to connect..." << endl;
                board.setType(CROSS);
                ingame = true;
                break;
            }
            case PLAYER_JOINED: {
                cout << "You successfull joined in game" << endl;
                board.setType(CIRCLE);
                ingame = true;
                break;
            }
            case SECOND_PLAYER_JOINED: {
                cout << "Second player to joined. Game start!" << endl;
                makeMove(board);
                break;
            }
            case MOVE: {
                if(!receiveInt(client_socket, &row))
                    serverDisconnected();

                if(!receiveInt(client_socket, &col))
                    serverDisconnected();

                board.otherMakeMove(row, col);

                game_over = makeMove(board);

                break;
            }
            case INVALID_COMMAND: {
                cout << "Invalid command! Please try again" << endl;
                break;
            }
            case GAMES_LIST: {
                temp = 'a';
                while (temp != '\0') {
                    if (read(client_socket, &temp, 1) > 0) {
                        cout << temp;
                    } else {
                        serverDisconnected();
                    }
                }
                break;
            }
            default: {
                cout << "Unrecognized response from the server" << endl;
                exit(1);
            }
        }
    }
}

int init_client(char* server_name) {
    int client_socket = 0;
    struct addrinfo *aip;
    struct addrinfo hint{};
    int err = 0;

    memset(&hint, 0, sizeof(hint));
    hint.ai_family = AF_INET;
    hint.ai_socktype = SOCK_STREAM;

    if ((err = getaddrinfo(server_name, PORT, &hint, &aip)) != 0) {
        cout << "Failed to get server address info" << gai_strerror(err) << endl;
        exit(1);
    }

    if ((client_socket = socket(aip->ai_family, aip->ai_socktype, 0)) < 0) {
        cout << "Failed to create socket" << endl;
        exit(1);
    }

    if (connect(client_socket, aip->ai_addr, aip->ai_addrlen) < 0) {
        cout << "Failed to connect to server" << endl;
        exit(1);
    }

    cout << "Connected to server successfully" << endl;

    return client_socket;
}

bool makeMove(Board &board) {
    bool game_over = false;
    int row = 0, col = 0;
    bool correct_input = false;

    int sizeBoard = board.getSize() - 1;

    board.DrawBoard();

    while (!correct_input) {
        cout << "Enter move(row col): ";

        cin >> row >> col;

        if (row < 0 || row > sizeBoard) {
            cout << "Invalid row input. Try again" << endl;
        } else if (col < 0 || col > sizeBoard) {
            cout << "Invalid col input. Try again" << endl;
        } else if (!board.isBlank(row, col)) {
            cout << "This cell is don't blank. Try again" << endl;
        } else {
            correct_input = true;
        }
    }

    board.playerMakeMove(row, col);

    if (!Status::sendStatus(client_socket, MOVE)) {
        serverDisconnected();
    }

    if (!sendInt(client_socket, row)) {
        serverDisconnected();
    }

    if (!sendInt(client_socket, col)) {
        serverDisconnected();
    }

    if (board.isWon()) {
        cout << "You win" << endl;
        game_over = true;
    } else {
        board.DrawBoard();
        cout << "wait other opponent..." << endl;
    }


    return game_over;
}

void serverDisconnected() {
    cout << "You've been disconnected from the server" << endl;
    close(client_socket);
    exit(1);
}