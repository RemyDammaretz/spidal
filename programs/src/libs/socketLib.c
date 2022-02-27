#include "../../include/check.h"
#include "../../include/socketLib.h"

// Setup socket and wait for connexion for Server
void serverConnect(int * se, int * sd, struct sockaddr_in * serverAddr, int serverPort) {
	// Setup TCP listening socket
	setupSocket(se, serverAddr, serverPort);
	
	// Waiting for connexion
	listen(*se, 1);
	*sd = accept(*se, NULL, NULL);
	CHECK(*sd, "--accept()");
}

// Setup socket and wait for connexion for Client
void clientConnect(int * sd, struct sockaddr_in * clientAddr, int clientPort, struct sockaddr_in * serverAddr, char * serverIP, int serverPort) {
	//int status = -1;
	
	// Setup TCP socket
	setupSocket(sd, clientAddr, clientPort);
	
	// Setup server address
	serverAddr->sin_family = AF_INET;
	serverAddr->sin_port = htons(serverPort);
	serverAddr->sin_addr.s_addr = inet_addr(serverIP);
	
	// Connexion
	// do {
	// 	status = connect(*sd, (const struct sockaddr *) serverAddr, sizeof(*serverAddr));
	// } while (status == -1);	
	while (connect(*sd, (const struct sockaddr *) serverAddr, sizeof(*serverAddr)) == -1) {
		sleep(1);
	}
}

// Function to setup a socket and bind an address
void setupSocket(int * sd, struct sockaddr_in * addr, int port) {
	int option = 1;
	
	// Setup TCP socket
	*sd = socket(AF_INET, SOCK_STREAM, 0);
	CHECK(*sd, "--socket()");
	
	setsockopt(*sd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
	
	// Def server addr
	addr->sin_family = AF_INET;
	addr->sin_port = htons(port);
	addr->sin_addr.s_addr = INADDR_ANY;
	
	// Bind address
	CHECK(bind(*sd, (const struct sockaddr *) addr, sizeof(*addr)), "--bind()");
}

// Function to send a message
int sendMessage(int sd, void * message, size_t size) {
	int status;
	
	status = send(sd, message, size, 0);
	return status == size;
}

// Function to wait for a message
int waitForMessage(int sd, void * message, size_t size) {
	int status;
	
	status = recv(sd, message, size, 0);
	return status == size;
}








