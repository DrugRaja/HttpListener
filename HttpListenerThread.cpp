#include "HttpListenerThread.h"
#include <mutex> 

evutil_socket_t HttpListenerThread::socket=-1;
bool HttpListenerThread::active = true;
std::mutex mtx;

void HttpListenerThread::operator() (unsigned int port, unsigned int threadID)
{
    printf("Thread %d listening on port %d... \n", threadID, port);

    std::unique_ptr<event_base, std::function<void(event_base *)>> eventBase(event_base_new(), &event_base_free);
    if(!eventBase)
    {
        printf("Thread %d ERROR: Failed to create event base!\n", threadID);
        return;
    }

    std::unique_ptr<evhttp, std::function<void(evhttp *)>> httpEvent(evhttp_new(eventBase.get()), &evhttp_free);
    if(!httpEvent)
    {
        printf("Thread %d ERROR: Failed to create http event handler!\n", threadID);
        return;
    }

    evhttp_set_gencb(httpEvent.get(), httpEventCallback, nullptr);

    mtx.lock();
    if(socket == -1)
    {
        boundSocket=evhttp_bind_socket_with_handle(httpEvent.get(), SERVER_IP, port);
        if(!boundSocket)
        {
            printf("Thread %d ERROR: Failed to bind socket with handle!\n", threadID);
            return;
        }
        if ((socket = evhttp_bound_socket_get_fd(boundSocket)) == -1)
        {
            printf("Thread %d ERROR: Failed to get fd!\n", threadID);
            return;
        }
    }
    else
    {
        if (evhttp_accept_socket(httpEvent.get(), socket) == -1)
        printf("Thread %d ERROR: Failed to bind socket.", threadID);
    }
    mtx.unlock();
    while(active)
    {
        event_base_loop(eventBase.get(), EVLOOP_NONBLOCK);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    printf("Thread %d EXIT OK\n", threadID);
}

void HttpListenerThread::httpEventCallback(evhttp_request * request, void * args)
{
    auto *OutBuf = evhttp_request_get_output_buffer(request);
      if (!OutBuf)
        return;
      evbuffer_add_printf(OutBuf, "<html><body><center><h1>Hello World!</h1></center></body></html>");
      evhttp_send_reply(request, HTTP_OK, "", OutBuf);
}

void HttpListenerThread::deactivate()
{
    HttpListenerThread::active = false;
}