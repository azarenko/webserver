#include "sockutils.h"

int readfromsock(int socket_fd, int len, const unsigned char* buffer, int utimeout)
{        
    int bytesRead = 0;
    int result;
    int exectime = 0;
    
    while (bytesRead < len)
    {
        result = recv(socket_fd, (void*)buffer + bytesRead, len - bytesRead, 0);
        if (result < 1 )
        {
            if(++exectime < utimeout)
            {
                usleep(1000);
            }
            else
            {
                return bytesRead;
            }
        }
        else
        {
            bytesRead += result;
        }
    }
    
    return bytesRead;
}
