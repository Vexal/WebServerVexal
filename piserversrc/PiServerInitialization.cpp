#include <iostream>
#include <string>
#include <fstream>
#include "PiServer.h"

int socketToKill = -1;
#ifndef _WIN32
#include<signal.h>
void sig_handler(int signo)
{
	using namespace std;
	if (signo == SIGINT)
	{
		cout << "Received sig kill" << endl;
		if (socketToKill > -1)
		{
			close(socketToKill);
		}
		exit(0);
	}
}

#endif
bool PiServer::initializeWSA()
{
#ifdef _WIN32
	WSADATA wsaData;

	// Initialize Winsock
	const int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		std::cout << "WSA initialization failed: " << iResult << std::endl;
		return false;
	}

	std::cout << "WSA initialization successful." << std::endl;
#endif
	return true;
}

bool PiServer::initializeTCPSocket()
{
#ifndef _WIN32
	if (signal(SIGINT, sig_handler) == SIG_ERR)
		std::cout << "ERROR: cannot catch signals." << std::endl;
#endif

	addrinfo *result = NULL, *ptr = NULL, hints;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the local address and port to be used by the server
	const auto iResult = getaddrinfo(NULL, this->port.c_str(), &hints, &result);
	if (iResult != 0)
	{
		std::cout << "getaddrinfo failed: " << iResult << std::endl;
#ifdef _WIN32
		WSACleanup();
#endif
		return false;
	}

	this->serverSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	socketToKill = this->serverSocket;
	if (this->serverSocket == INVALID_SOCKET)
	{
		std::cout << "Error at socket(): " << std::endl;
		freeaddrinfo(result);
#ifdef _WIN32
		WSACleanup();
#endif
		return false;
	}

#ifdef _WIN32
	unsigned long timeout = 20000; //60 seconds
#else
	struct timeval timeout;
	timeout.tv_sec = 20;
	timeout.tv_usec = 0;
#endif
	setsockopt(this->serverSocket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));

	std::cout << "Socket creation successful." << std::endl;

	const auto iResult2 = bind(this->serverSocket, result->ai_addr, result->ai_addrlen);

	if (iResult2 == SOCKET_ERROR)
	{
		std::cout << "bind() failed with error: " << std::endl;
		freeaddrinfo(result);
#ifdef _WIN32
		closesocket(this->serverSocket);
		WSACleanup();
#else
		close(this->serverSocket);
#endif
		return false;
	}

	std::cout << "Socket binding successful." << std::endl;
	freeaddrinfo(result);
	return true;
}

bool PiServer::listenSocket()
{
	const auto res = listen(this->serverSocket, SOMAXCONN);

	//u_long iMode = 1;
	//ioctlsocket(this->serverSocket, FIONBIO, &iMode);
	if (res == SOCKET_ERROR)
	{
		std::cout << "Listen failed with error: " << std::endl;
#ifdef _WIN32
		closesocket(this->serverSocket);
		WSACleanup();
#else
		close(this->serverSocket);
#endif
		return false;
	}

	std::cout << "Listen socket successful." << std::endl;
	return true;
}

void closesocket2(SOCKET socket)
{
#ifdef _WIN32
	closesocket(socket);
#else
	close(socket);
#endif
}
