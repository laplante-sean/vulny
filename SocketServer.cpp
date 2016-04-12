#include "SocketServer.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>

using namespace std;

SocketServer::SocketServer(uint16_t p) :
	port(p), running(false)
{}

SocketServer::~SocketServer() {
	//TODO: Close stuff
}

int SocketServer::run() {
	int sockfd, newsockfd;
	socklen_t clilen;
	char buffer[256];
	struct sockaddr_in serv_addr, cli_addr;
	int n;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		cerr << "ERROR: opening socket\n";
		return 1;
	}

	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(port);

	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		  cerr << "ERROR: Could not bind to port: " << port <<"\n";
		  return 1;
	}

	listen(sockfd,5);
	clilen = sizeof(cli_addr);
	newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
	if (newsockfd < 0)
	  cerr << "ERROR: on accept\n";
	bzero(buffer,256);
	n = read(newsockfd,buffer,255);
	if (n < 0) {
		cerr << "ERROR: reading from socket\n";
	}
	printf("Here is the message: %s\n",buffer);
	n = write(newsockfd,"I got your message",18);
	if (n < 0) {
		cerr << "ERROR: writing to socket\n";
	}

	close(newsockfd);
	close(sockfd);
	return 0;
}
void SocketServer::stop() {
	running = false;
}

bool SocketServer::isRunning() {
	return running;
}

