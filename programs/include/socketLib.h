#ifndef SOCKET_LIB
#define SOCKET_LIB

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

// Server mode
void serverConnect(int * se, int * sd, struct sockaddr_in * serverAddr, int port);


// Client mode
void clientConnect(int * sd, struct sockaddr_in * clientAddr, int clientPort, struct sockaddr_in * serverAddr, char * serverIP, int serverPort);

// -------

void setupSocket(int * sd, struct sockaddr_in * addr, int port);

int sendMessage(int sd, void * message, size_t size);
int waitForMessage(int sd, void * message, size_t size);


#endif

