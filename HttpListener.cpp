// HttpListener.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "HttpListenerThread.h"

//int parsePortParameter(int argc, char* argv[])
//{
//   int inputPort = 0;
//
//   if ((argc > 1) && (argv[1][0] == '-') && (std::tolower(argv[1][1]) == 'p'))
//   {
//      if (argc == 3)
//      {
//         inputPort = atoi(argv[2]);
//      }
//      else if (argc == 2)
//      {
//         inputPort = atoi(&argv[1][2]);
//      }
//
//      // Check port input
//      if ((inputPort >= 0) && (inputPort <= 65535))
//      {
//         return (unsigned int)inputPort;
//      }
//      else
//      {
//         printf("Invalid port value entered. Will use default port: %d\n\n", DEFAULT_LISTENING_PORT);
//         return -1;
//      }
//   }
//   else
//   {
//      printf("Port parameter missing. Using default: %d\n\n", DEFAULT_LISTENING_PORT);
//      return -1;
//   }
//}

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

   std::thread* threads[DEFAULT_THREAD_NUMBER];
   int rc=-1;

   for (unsigned int i=0;i<DEFAULT_THREAD_NUMBER;i++)
   {
      std::thread* currentThread(new std::thread(HttpListenerThread(), listenerPort));
      threads[i] = currentThread;

   }
   printf("created...\n");
   std::this_thread::sleep_for(std::chrono::milliseconds(3000));
   return 0;
}

