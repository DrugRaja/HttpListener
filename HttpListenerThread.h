#pragma once
#include "stdafx.h"
#include <evhttp.h>

class HttpListenerThread
{
public:
    void operator() (unsigned int port, unsigned int threadID);
    static void httpEventCallback(evhttp_request * request, void * args);
    static void deactivate();
private:
   static bool active;
   static evutil_socket_t socket;
   evhttp_bound_socket* boundSocket;
};