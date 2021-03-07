#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netdb.h>
#include <cstring>
#include "Board.h"
#include "Status.h"
#include "sockets.h"
#include "CellType.h"

#define BUF_SIZE 1024

using namespace std;

bool connect_to_socket(char* server_name, const string& port, char* client_name);
void serverDisconnected();
bool makeMove();
void menu(char* buffer);

enum Menu { LIST = 1, NEW_GAME = 2, JOIN_GAME = 3 };
int client_socket = 0;
char *server_name;
char *client_name;
string portList[3] = {"50001", "50002", "50003"};
int currentIndexPort = 0;

Board *board;

int main(int argc, char* argv[]) {
    char buffer[BUF_SIZE];
    int row, col;
    bool game_over = false;
    bool ingame = false;
    char temp;
    server_name = argv[1];
    client_name = argv[2];
    StatusCode statusCode;

    if (argc != 3) {
        cout << "error, enter server address" << endl;
        exit(1);
    }

    for (int i = 0; i < portList->length(); ++i) {
        bool socketIsConnected = connect_to_socket(server_name, portList[i], client_name);
        currentIndexPort++;
        if (socketIsConnected)
            break;
    }

    while (!game_over) {
        if (!receiveStatus(client_socket, &statusCode)) {
            serverDisconnected();
            if (!receiveStatus(client_socket, &statusCode)) {
                serverDisconnected();
            }
        }

        switch (statusCode) {
            case REGISTERED: {
                cout << "Player " << argv[2] << " registered!" << endl;
                break;
            }
            case CREATED: {
                cout << "Game created! Waiting your opponent to connect..." << endl;
                board = new Board();
                board->setType(CROSS);
                ingame = true;
                break;
            }
            case PLAYER_JOINED: {
                cout << "You successfull joined in game" << endl;
                board = new Board();
                board->setType(CIRCLE);
                ingame = true;
                break;
            }
            case SECOND_PLAYER_JOINED: {
                cout << "Second player to joined. Game start!" << endl;
                board->DrawBoard();
                makeMove();
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

                board->otherMakeMove(row, col);
                board->DrawBoard();

                game_over = makeMove();

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
                sendStatus(client_socket, SWITCH_TO_COMMAND);
                cout << "Press Enter for continue...";
                int c = getchar();
                while ((c = getchar()) != '\n' && c != EOF) {}
                ingame = false;
                break;
            }
            case STATE: {
                for (int i = 0; i < MAX_ROWS; ++i) {
                    for (int j = 0; j < MAX_ROWS; ++j) {
                        int cell;
                        receiveInt(client_socket, &cell);
                        board->board[i][j] = cell;
                    }
                }

                board->DrawBoard();

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

bool connect_to_socket(char* server_name, const string& port, char* client_name) {
    struct addrinfo *aip;
    struct addrinfo hint{};
    int err = 0;

    memset(&hint, 0, sizeof(hint));
    hint.ai_family = AF_INET;
    hint.ai_socktype = SOCK_STREAM;

    if ((err = getaddrinfo(server_name, port.c_str(), &hint, &aip)) != 0) {
        cout << "Failed to get server address info: " << gai_strerror(err) << endl;
        return false;
    }

    if ((client_socket = socket(aip->ai_family, aip->ai_socktype, 0)) < 0) {
        cout << "Failed to create socket" << endl;
        return false;
    }

    if (connect(client_socket, aip->ai_addr, aip->ai_addrlen) < 0) {
        cout << "Failed to connect to server" << endl;
        return false;
    }

    cout << "Connected to server successfully" << endl;
    char buffer[BUF_SIZE];

    snprintf(buffer, BUF_SIZE, "register %s\n", client_name);
    write(client_socket, buffer, strlen(buffer));

    return true;
}

bool makeMove() {
    bool game_over = false;
    int row = 0, col = 0;
    bool correct_input = false;

    while (!correct_input) {
        cout << "Enter move(row col): ";

        cin >> row >> col;

        if (row < 0 || row > MAX_ROWS) {
            cout << "Invalid row input. Try again" << endl;
        } else if (col < 0 || col > MAX_ROWS) {
            cout << "Invalid col input. Try again" << endl;
        } else if (!board->isBlank(row, col)) {
            cout << "This cell is don't blank. Try again" << endl;
        } else {
            correct_input = true;
        }
    }

    if (!sendStatus(client_socket, MOVE)) {
        serverDisconnected();
    }

    if (!sendInt(client_socket, row)) {
        serverDisconnected();
    }

    if (!sendInt(client_socket, col)) {
        serverDisconnected();
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
            snprintf(buffer, BUF_SIZE - 1, "join %s\n", buf);
            write(client_socket, buffer, strlen(buffer) - 1);
            break;
        case JOIN_GAME:
            cout << "Enter join game name: ";
            fgets(buf, BUF_SIZE - 1, stdin);
            snprintf(buffer, BUF_SIZE - 1, "join %s\n", buf);
            write(client_socket, buffer, strlen(buffer) - 1);
            break;
        default:
            break;
    }
}

void serverDisconnected() {
    cout << "You've been disconnected from the server" << endl;
    close(client_socket);

    for (int i = currentIndexPort; i < portList->length(); ++i) {
        bool socketIsConnected = connect_to_socket(server_name, portList[i], client_name);
        currentIndexPort++;
        if (socketIsConnected)
            break;
    }
}