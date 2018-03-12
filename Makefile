CC=g++
CFLAGS=-I. -I../libevent/include -std=c++11 -pthread -g -Wall 
DEPS = HttpListenerThread.h HttpListenerDefinitions.h stdafx.h
OBJ = HttpListener.o stdafx.o HttpListenerThread.o
LIBS = -levent -laws-cpp-sdk-core -laws-cpp-sdk-s3
LDIR = 

%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

HttpListener: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LDIR) $(LIBS)