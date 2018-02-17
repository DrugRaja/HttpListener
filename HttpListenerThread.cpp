#include "HttpListenerThread.h"
#include <mutex> 

evutil_socket_t HttpListenerThread::socket=-1;
bool HttpListenerThread::active = true;
std::mutex mtx;

void HttpListenerThread::operator() (unsigned int port, unsigned int threadID)
{
    printf("Thread %d listening on port %d... \n", threadID, port);

    // Created libevent event base
    std::unique_ptr<event_base, std::function<void(event_base *)>> eventBase(event_base_new(), &event_base_free);
    if(!eventBase)
    {
        printf("Thread %d ERROR: Failed to create event base!\n", threadID);
        return;
    }

    // Create libevent HTTP request handler
    std::unique_ptr<evhttp, std::function<void(evhttp *)>> httpEvent(evhttp_new(eventBase.get()), &evhttp_free);
    if(!httpEvent)
    {
        printf("Thread %d ERROR: Failed to create http event handler!\n", threadID);
        return;
    }

    // Register specific URI and general HTTP callback functions.
    evhttp_set_cb(httpEvent.get(), "/wav-info", httpWavInfoCallback, nullptr);
    evhttp_set_cb(httpEvent.get(), "/mp3-to-wav", httpMp3ToWavCallback, nullptr);
    evhttp_set_gencb(httpEvent.get(), httpEventCallback, nullptr);

    // Bind to listening socket
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

    // Check if there are HTTP requests
    while(active)
    {
        event_base_loop(eventBase.get(), EVLOOP_NONBLOCK);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    printf("Thread %d EXIT OK\n", threadID);
}

void HttpListenerThread::deactivate()
{
    HttpListenerThread::active = false;
}

void HttpListenerThread::httpEventCallback(evhttp_request * request, void * args)
{
    evbuffer *outputBuffer = evhttp_request_get_output_buffer(request);
    if (outputBuffer)
    {
        evbuffer_add_printf(outputBuffer, "<html><body><center><h1>Request received. Processing not available for this type of request.</h1></center></body></html>");
        evhttp_send_reply(request, HTTP_OK, "", outputBuffer);
    }
}

void HttpListenerThread::httpWavInfoCallback(evhttp_request * request, void * args)
{
    evbuffer *outputBuffer = evhttp_request_get_output_buffer(request);
    if (outputBuffer)
    {
        evbuffer_add_printf(outputBuffer, "<html><body><center><h1>WAV INFO</h1></center></body></html>");
        evhttp_send_reply(request, HTTP_OK, "", outputBuffer);
    }
}

void HttpListenerThread::httpMp3ToWavCallback(evhttp_request * request, void * args)
{
    evbuffer *outputBuffer = evhttp_request_get_output_buffer(request);
    if (outputBuffer)
    {
        evbuffer_add_printf(outputBuffer, "<html><body><center><h1>MP3 to WAV</h1></center></body></html>");
        evhttp_send_reply(request, HTTP_OK, "", outputBuffer);
    }
}