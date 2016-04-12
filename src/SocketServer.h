#ifndef SOCKET_SERVER_H_
#define SOCKET_SERVER_H_

#include <stdint.h>
#include <thread>
#include <mutex>
#include <iostream>

class SocketServer {
public:
	SocketServer(uint16_t p);
	~SocketServer();
public:
	int run();
	void stop();
private:
	void handleConnection(int c);
	bool isRunning();
private:
	uint16_t port;
	bool running;
	int sock;
	std::mutex objLock;
};

#endif /* SOCKET_SERVER_H_ */
