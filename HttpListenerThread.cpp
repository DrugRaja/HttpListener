#include "HttpListenerThread.h"

void HttpListenerThread::operator() (unsigned int port)
{
    printf("Listening at %d... \n", port);
    if (!event_init())
    {
        printf("libevent error!\n");
    }
    else
    {
        printf("libevent success!\n");
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
}