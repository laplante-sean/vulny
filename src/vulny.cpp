#include "SocketServer.h"
#include <iostream>
#include <stdint.h>
#include <boost/lexical_cast.hpp>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

using namespace std;

static SocketServer* s;

void exit_handler(int sig) {
	cout << "Shutting down\n";
	if (s) {
		s->stop();
		delete s;
	}
	exit(0);
}

int main(int argc, char ** argv) {
	struct sigaction sigIntHandler;

	sigIntHandler.sa_handler = exit_handler;
	sigemptyset(&sigIntHandler.sa_mask);
	sigIntHandler.sa_flags = 0;

	sigaction(SIGINT, &sigIntHandler, NULL);

	if (argc < 2) {
		cerr << "No port provided\n";
		cout << "Usage: " << argv[0] << " <port>\n";
		return 1;
	}
	
	uint16_t port = 0;
	try {
		port = boost::lexical_cast<uint16_t>(argv[1]);
	} catch (boost::bad_lexical_cast& ex) {
		cerr << "Invalid port number: " << argv[1] << ". " << ex.what() << "\n";
		return 1;
	}

	s = new SocketServer(port);
	int ret = s->run();
	delete s;
	return ret;
}
