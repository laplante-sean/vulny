#include <iostream>
#include <stdint.h>
#include <boost/lexical_cast.hpp>

using namespace std;

class SocketServer;

int main(int argc, char ** argv) {
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

	SocketServer s = SocketServer(port);
	return s.run();
}
