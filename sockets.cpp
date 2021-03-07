#include "sockets.h"

bool sendInt(int socket, int number) {
    char * data = (char*)&number;
    size_t left = sizeof(number);
    ssize_t rc;

    while (left) {
        rc = write(socket, data + sizeof(number) - left, left);
        if (rc <= 0) return false;
        left -= rc;
    }

    return true;
}

bool receiveInt(int socket, int * number) {
    int ret;
    char *data = (char*)&ret;
    size_t left = sizeof(*number);
    ssize_t rc;

    while (left) {
        rc = read(socket, data + sizeof(ret) - left, left);
        if (rc <= 0) return false;
        left -= rc;
    }

    *number = ret;
    return true;
}

int init_server(char *port) {
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

    if ((err = getaddrinfo(hostname, port, &hint, &aip)) != 0) {
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

    if (listen(orig_socket, 100) < 0) {
        perror("Failed to start listening");
        close(orig_socket);
        exit(1);
    }

    return orig_socket;
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
