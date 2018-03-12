#pragma once
#include "stdafx.h"
#include <evhttp.h>

class HttpListenerThread
{
public:
    void operator() (unsigned int port, unsigned int threadID);
    void httpEventCallback(evhttp_request * request, void * args);
    void httpWavInfoCallback(evhttp_request * request, void * args);
    void httpMp3ToWavCallback(evhttp_request * request, void * args);
    static void deactivate();
private:
   static bool active;
   static evutil_socket_t socket;
   evhttp_bound_socket* boundSocket;

   std::string parseKey(const std::string& uri, const std::string& keyname);
   std::string executeCommand(const char* cmd);
   std::string getFileName(std::string& keyName);
   int getFile(const std::string& bucketName, std::string& keyName, std::string& result);
   int uploadFile(const std::string& bucketName, const std::string& keyName, std::string& wavFileName, std::string& result);
};