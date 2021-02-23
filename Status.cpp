#include "Status.h"

bool Status::sendStatus(int socket, StatusCode statusCode) {
    char *data = (char*)&statusCode;
    size_t left = sizeof(statusCode);
    ssize_t rc;

    while (left) {
        rc = write(socket, data + sizeof(statusCode) - left, left);
        if (rc <= 0) {
            return false;
        }

        left -= rc;
    }

    return true;
}

bool Status::receiveStatus(int socket, StatusCode *statusCode) {
    StatusCode ret;
    char *data = (char*)&ret;
    size_t left = sizeof(*statusCode);
    ssize_t rc;

    while (left) {
        rc = read(socket, data + sizeof(ret) - left, left);
        if (rc <= 0) {
            return false;
        }

        left -= rc;
    }

    *statusCode = ret;
    return true;
}
