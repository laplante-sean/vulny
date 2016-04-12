#ifndef SOCKET_SERVER_H_
#define SOCKET_SERVER_H_

#include <stdint.h>

class SocketServer {
public:
	SocketServer(uint16_t p);
	~SocketServer();
public:
	int run();
	void stop();
private:
	bool isRunning();
private:
	uint16_t port;
	bool running;
};

#endif /* SOCKET_SERVER_H_ */
