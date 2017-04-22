#include "SocketServer.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <memory>
#include <unordered_map>
#include <set>
#include <string>
#include <stdexcept>
#include <array>
#include <iostream>
#include <cstdio>

#include <boost/algorithm/string.hpp>

using namespace std;

static uint64_t connectionId = 0;

struct connection {
	int sock_fd;
	std::thread * t;
};

static std::string exec(const char * cmd) {
	std::array<char, 128>buffer;
	std::string result;
	std::shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
	if (!pipe) throw std::runtime_error("popen() failed!");
	while(!feof(pipe.get())) {
		if (fgets(buffer.data(), 128, pipe.get()) != NULL)
			result += buffer.data();
	}
	return result;
}

class SocketServerImpl {
public:
	SocketServerImpl() :
		port(), sock(), running(true), activeConnections() {
		cleanupThread = std::thread(&SocketServerImpl::cleanup, this);
	}
	~SocketServerImpl(){
		stop();
		cleanupThread.join();
		for (auto conn : activeConnections) {
			uint64_t id = conn.first;
			connection c = conn.second;
			shutdown(c.sock_fd, SHUT_RDWR);
			c.t->join();
			delete c.t;
		}
		activeConnections.clear();
		shutdown(sock,SHUT_RDWR);
		close(sock);
	}
public:
	void addConnection(int sock_fd, std::thread * t, uint64_t id) {
		std::lock_guard<std::mutex> lock(objLock);
		connection c;
		c.sock_fd = sock_fd;
		c.t = t;
		activeConnections[id] = c;
	}
	void addInactiveConnection(uint64_t id) {
		std::lock_guard<std::mutex> lock(objLock);
		inactiveConnections.insert(id);
	}
	bool isRunning() const {
		return running;
	}
	void stop() {
		std::lock_guard<std::mutex> lock(objLock);
		running = false;
	}
private:
	void cleanup() {
		cout << "Cleanup thread running\n";
		while(isRunning()) {
			objLock.lock();
			if (inactiveConnections.size()) {
				for (uint64_t id : inactiveConnections) {
					cout << "Found connection: " << id << " for cleanup\n";
					connection c = activeConnections[id];
					if(c.t->joinable())
						c.t->join();
					delete c.t;
					activeConnections.erase(id);
					cout << "Connection " << id << " cleanup complete\n";
				}
				inactiveConnections.clear();
			}
			objLock.unlock();
			sleep(1);
		}
		cout << "Cleanup thread exit\n";
	}
public:
	uint16_t port;
	int sock;
private:
	bool running;
	std::mutex objLock;
	std::unordered_map<uint64_t,connection> activeConnections;
	std::set<uint64_t> inactiveConnections;
	std::thread cleanupThread;
};

SocketServer::SocketServer(uint16_t p) :
		pImpl(new SocketServerImpl()) {
	pImpl->port = p;
}

SocketServer::~SocketServer() {
	delete pImpl;
}

int SocketServer::run() {
	struct sockaddr_in server;

	pImpl->sock = socket(AF_INET, SOCK_STREAM, 0);
	if (pImpl->sock == -1) {
		cerr << "Could not create socket\n";
		return 1;
	}

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(pImpl->port);

	if (bind(pImpl->sock,(struct sockaddr*)&server, sizeof(server)) < 0) {
		cerr << "Could not bind to port: " << pImpl->port << "\n";
		close(pImpl->sock);
		return 1;
	}

	listen(pImpl->sock, 15);

	while(pImpl->isRunning()) {
		uint32_t size = sizeof(struct sockaddr_in);
		struct sockaddr_in client;

		cout << "Waiting for new connection\n";
		int newsock = accept(pImpl->sock, (struct sockaddr*)&client, &size);

		if (newsock != -1 && newsock != 0) {
			pImpl->addConnection(
				newsock,
				new std::thread(&SocketServer::handleConnection, this, newsock, connectionId),
				connectionId
			);
			connectionId++;
		} else {
			cout << "Error on accept(). Exit now\n";
			break;
		}
	}

	cout << "SocketServer Exit";
	close(pImpl->sock);
	return 0;
}

void SocketServer::handleConnection(int sock_fd, uint64_t id) {
	cout << "New connection " << id << "\n";

	const static char * msg = "Enter a host to ping: ";
	char recvBuf[1025] = {'\0'};
	while(pImpl->isRunning()) {
		memset(recvBuf, 0, sizeof(recvBuf));
		if (send(sock_fd,msg,strlen(msg),0) == -1) {
			cerr << "Failed to send data for connection " << id << "\n";
			break;
		}
		
		int recvd = recv(sock_fd,recvBuf,sizeof(recvBuf),0);
		
		if (recvd == 0)
			break;
		else if (recvd == -1) {
			cerr << "Failed to receive data for connection " << id << "\n";
			break;
		}

		string host(recvBuf);
		vector<string> parts;
		boost::split(parts, host, boost::is_space(), boost::token_compress_on);

		bool sneaky = false;
		bool extra_sneaky = false;
		for (string& part : parts) {
			boost::trim(part);

			//Super secure string safety
			//ping, ls, and cd and some others
			//are the only commands that can be used for compromise. I'm sure of it!
			if (boost::contains(part,"ping") ||
				boost::contains(part,"ls") ||
				boost::contains(part,"cd") ||
				boost::contains(part,"nc") ||
				boost::contains(part,"ncat") ||
				boost::contains(part,"ruby")) {
				sneaky = true;
			} else if(part.compare(";") == 0) {
				extra_sneaky = true;
			}
		}

		//Throw and & on the end so they don't hack the system. Super secure.
		string cmd = "ping -c 3 ";

		//put it back together
		for (const string& part : parts) {
			cmd += part + " ";
		}
		//cmd += "&";
		boost::trim(cmd);

		cout << cmd << "\n";

		if (extra_sneaky) {
			cout << "Extra sneaky\n";
			const static char * extra_dumby = "So, you wanna run more than one command and you think a little ';' is gonna help you. WRONG!! YOU LOOSE! GOOD DAY SIR!\n";
			send(sock_fd, extra_dumby, strlen(extra_dumby), 0);
		}else if (sneaky) {
			cout << "Sneaky\n";
			const static char * dumby = "No! Just a host! Don't try commands. Don't try to be sneaky...you're not. This is real life. Stop hacking! https://github.com/vix597/vulny\n";
			send(sock_fd, dumby, strlen(dumby), 0);
		} else {
			//int status = system(cmd.c_str());

			std::string result = exec(cmd.c_str());
			send(sock_fd, result.c_str(), result.length(), 0);
			//if (status == 0) {
				//cout << "Success\n";
				//const static char * success = "Command success. Congratulation on using ping! You are a true master of the computer.\n";
			//	send(sock_fd, success, strlen(success), 0);
			//} else {
			//	cout << "Fail\n";
			//	const static char * fail = "Command failure\n";
			//	send(sock_fd, fail, strlen(fail), 0);
			//}
		}
	}

	cout << "Connection " << id << " closed\n";
	pImpl->addInactiveConnection(id);
	shutdown(sock_fd, SHUT_RDWR);
	close(sock_fd);
	return;
}
