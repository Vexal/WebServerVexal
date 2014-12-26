#include "Client.h"

using namespace std;

Client::Client(int socket, const sockaddr& address, const string& addressString) :
	address(address),
	socket(socket),
	addressString(addressString)
{

}
