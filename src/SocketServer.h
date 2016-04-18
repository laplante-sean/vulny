#ifndef SOCKET_SERVER_H_
#define SOCKET_SERVER_H_

#include <stdint.h>
#include <thread>
#include <mutex>
#include <iostream>

class SocketServerImpl;

class SocketServer {
public:
	SocketServer(uint16_t p);
	~SocketServer();
public:
	int run();
private:
	void handleConnection(int c, uint64_t id);
private:
	SocketServerImpl * pImpl;
};

#endif /* SOCKET_SERVER_H_ */
