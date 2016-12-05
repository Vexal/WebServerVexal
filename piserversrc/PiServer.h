#pragma once
#include <string>
#include <map>
#include <queue>
#include <mutex>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifdef _WIN32
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
#include <string>


#include <netdb.h>

#include <unistd.h>
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define SOCKET int
#endif
#include "PiController.h"

class PiServer
{
private:
	SOCKET serverSocket = INVALID_SOCKET;
	std::string port = "9191";
	PiController piController;

public:
	bool InitializeServer();

	void Update();

private:
	bool initializeWSA();
	bool initializeTCPSocket();
	bool listenSocket();

	bool checkForNewConnection();
	void receiveThenHandleClientRequest(SOCKET clientSocket, const std::string& clientAddressString);
};

void closesocket2(SOCKET);