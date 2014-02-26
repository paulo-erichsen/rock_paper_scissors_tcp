CC=g++

all: server client

server : server.o helpers.o
	$(CC) server.o helpers.o -o server

client : client.o helpers.o
	$(CC) client.o helpers.o -o client

server.o : server.cpp server.h
	$(CC) -c server.cpp

client.o : client.cpp client.h
	$(CC) -c client.cpp

helpers.o : helpers.cpp helpers.h constants.h
	$(CC) -c helpers.cpp

clean :
	rm -rf *o client server
