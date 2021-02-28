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

void init_client(char* server_name);
void serverDisconnected();
bool makeMove(Board &board);
void menu(char* buffer);

enum Menu { LIST = 1, NEW_GAME = 2, JOIN_GAME = 3 };
int client_socket = 0;

int main(int argc, char* argv[]) {
    char buffer[BUF_SIZE];
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

    init_client(argv[1]);

    snprintf(buffer, BUF_SIZE, "register %s\n", argv[2]);
    write(client_socket, buffer, strlen(buffer));

    while (!game_over) {
        if (!Status::receiveStatus(client_socket, &statusCode)) {
            serverDisconnected();
        }

        switch (statusCode) {
            case REGISTERED: {
                cout << "Player " << argv[2] << " registered!" << endl;
                break;
            }
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
            case GAME_EXIST: {
                cout << "Game exist. Choose another game" << endl;
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
            case WIN: {
                cout << "You win!" << endl;
                cout << "Press Enter for continue...";
                int c = getchar();
                while ((c = getchar()) != '\n' && c != EOF) {}
                ingame = false;
                break;
            }
            case LOSS: {
                cout << "You loss!" << endl;
                cout << "Press Enter for continue...";
                int c = getchar();
                while ((c = getchar()) != '\n' && c != EOF) {}
                ingame = false;
                break;
            }
            case WRONG: {
                cout << "You wrong!" << endl;
                break;
            }
            default: {
                cout << "Unrecognized response from the server" << endl;
                exit(1);
            }
        }

        if (!ingame) {
            menu(buffer);
        }
    }
}

void init_client(char* server_name) {
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

    board.DrawBoard();

    if (board.isWon()) {
        cout << "wait opponent..." << endl;
    }


    return game_over;
}

void menu(char* buffer) {
    cout << "1. Render list games" << endl;
    cout << "2. Create new game" << endl;
    cout << "3. Join the game" << endl;
    cout << "Choose command: ";

    fgets(buffer, BUF_SIZE - 1, stdin);

    int i;
    fflush(stdin);
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
            write(client_socket, buffer, strlen(buffer) - 1);
            break;
        case JOIN_GAME:
            cout << "Enter join game name: ";
            fgets(buf, BUF_SIZE - 1, stdin);
            snprintf(buffer, BUF_SIZE, "join %s\n", buf);
            write(client_socket, buffer, strlen(buffer) - 1);
        default:
            break;
    }
}

void serverDisconnected() {
    cout << "You've been disconnected from the server" << endl;
    close(client_socket);
    exit(1);
}