#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int readfromsock(int socket_fd, int len, const unsigned char* buffer, int utimeout);
