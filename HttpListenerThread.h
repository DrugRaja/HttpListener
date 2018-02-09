#pragma once
#include "stdafx.h"

class HttpListenerThread
{
public:
   HttpListenerThread() {};
   void operator() (unsigned int port)
   {
      printf("Listening at %d... \n", port);
      std::this_thread::sleep_for(std::chrono::milliseconds(2000));
   }
};