#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <cstring>
#include <bits/confname.h>
#include <zconf.h>

#define PORT "50002"
#define MAX_CLIENTS 10

using namespace std;

int init_server();
char* allocate_hostname();
void* handle_client(void* arg);

int server_socket = 0;

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

    /* to be continued */
}