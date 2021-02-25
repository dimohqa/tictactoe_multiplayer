#include "sockets.h"

bool sendInt(int socket, int number)
{
    char * data = (char*)&number;
    size_t left = sizeof(number);
    ssize_t rc;

    while (left)
    {
        rc = write(socket, data + sizeof(number) - left, left);
        if(rc <= 0) return false;
        left -= rc;
    }

    return true;
}

bool receiveInt(int socket, int * number)
{
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
