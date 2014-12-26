#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <string>
#ifdef WIN32
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#endif
#include <vector>

struct ClientRequest
{
	std::string referer;
	std::string refererDomain;
	std::string requestCommand;
	std::string userAgent;
	std::string requestTarget;
	std::string requestTime;
	std::string hostName;
};

class Client
{
private:
	const sockaddr address;
	const std::string addressString;
	const int socket;
	std::vector<ClientRequest> requests;

public:
	Client(int socket, const sockaddr& address, const std::string& addressString);

	sockaddr GetAddress() const {return this->address;}
	std::string GetAddressString() const {return this->addressString;}
	void AddClientRequest(const ClientRequest& request) {this->requests.push_back(request);}

private:
	int GetSocket() const {return this->socket;}
};