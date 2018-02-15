// HttpListener.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "HttpListenerThread.h"

struct ThreadDeleter {
   void operator()(std::thread* t) 
   {
      if(t)
      {
         t->join();
         delete t;
      }
   }
};

int main(int argc, char* argv[])
{
    int inputPort = 0;
    unsigned int listenerPort = DEFAULT_LISTENING_PORT;

    if ((argc > 1) && (argv[1][0] == '-') && (std::tolower(argv[1][1]) == 'p'))
    {
        if (argc == 3)
        {
            inputPort = atoi(argv[2]);
        }
        else if (argc == 2)
        {
            inputPort = atoi(&argv[1][2]);
        }
        // Check port input
        if ((inputPort >= 0) && (inputPort <= 65535))
        {
            listenerPort = (unsigned int)inputPort;
        }
        else
        {
            printf("Invalid port value entered. Will use default port: %d\n\n", listenerPort);
        }
    }
    else 
    {
        printf("Port parameter missing. Using default: %d\n\n", listenerPort);
    }

    typedef std::unique_ptr<std::thread, ThreadDeleter> ThreadPointer;
    std::vector<ThreadPointer> threadPool;

    for (unsigned int i=0;i<DEFAULT_THREAD_NUMBER;i++)
    {
        ThreadPointer thread(new std::thread(HttpListenerThread(), listenerPort, i));
        threadPool.push_back(std::move(thread));
    }

    std::cout << "Press Enter to quit.\n" << std::endl;
    std::cin.get();
    HttpListenerThread::deactivate();
    return 0;
}

