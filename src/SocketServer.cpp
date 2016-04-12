#include "SocketServer.h"
#include "Game.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

using namespace std;

SocketServer::SocketServer(uint16_t p) :
	port(p), running(true), sock(0)
{}

SocketServer::~SocketServer() {
	if (isRunning())
		stop();
	if (sock != -1)
		close(sock);
}

int SocketServer::run() {
	struct sockaddr_in server;

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1) {
		cerr << "Could not create socket\n";
		return 1;
	}

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(port);

	if (bind(sock,(struct sockaddr*)&server, sizeof(server)) < 0) {
		cerr << "Could not bind to port: " << port << "\n";
		close(sock);
		return 1;
	}

	listen(sock, 15);

	while(isRunning()) {
		uint32_t size = sizeof(struct sockaddr_in);
		struct sockaddr_in client;

		cout << "Waiting for new connection\n";
		int newsock = accept(sock, (struct sockaddr*)&client, &size);

		if (newsock != -1) {
			std::thread(&SocketServer::handleConnection, this, newsock);
		}
	}

	close(sock);
	return 0;
}

void SocketServer::stop() {
	std::lock_guard<std::mutex> lock(objLock);
	running = false;
}

bool SocketServer::isRunning() {
	std::lock_guard<std::mutex> lock(objLock);
	return running;
}

void SocketServer::handleConnection(int c) {
	while(isRunning()) {
		cout << "Thread running\n";
	}
}

