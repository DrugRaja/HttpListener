#pragma once
#include "stdafx.h"
#include <evhttp.h>

class HttpListenerThread
{
public:
   HttpListenerThread() {};
   void operator() (unsigned int port);
};