#include <iostream>
#include "PiServer.h"

#ifdef _WIN32
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "winmm.lib")
#else
#include <time.h>
#endif

using namespace std;

static const bool printEverything = true;
int main()
{
	PiServer server;
	server.InitializeServer();
	while (true)
	{
		server.Update();
	}

	return 0;
}

bool PiServer::InitializeServer()
{
	this->initializeWSA();
	cout << "Initializing Server Socket..." << endl;
	this->initializeTCPSocket();
	cout << "Listening Server Socket..." << endl;
	this->listenSocket();

	cout << endl << "Initialization successful." << endl;
	return true;
}

void PiServer::Update()
{
	this->checkForNewConnection();
}

bool PiServer::checkForNewConnection()
{
	sockaddr clientAddress;
	socklen_t clientAddressLength = sizeof(clientAddress);
	fd_set readSet;
	FD_ZERO(&readSet);
	FD_SET(this->serverSocket, &readSet);

	//block here until a connection is found.
	const auto clientSocket = accept(this->serverSocket, &clientAddress, &clientAddressLength);

	if (clientSocket < 0 || clientSocket == INVALID_SOCKET)
	{
		cout << "inv" << endl;
		closesocket2(clientSocket);
		return false;
	}

	const sockaddr_in* const add2 = reinterpret_cast<sockaddr_in*>(&clientAddress);

	const string clientAddressString(inet_ntoa(add2->sin_addr));
	if (printEverything)
		cout << "Received connection from client with address " << clientAddressString << endl;

	this->receiveThenHandleClientRequest(clientSocket, clientAddressString);
	return true;
}

#define MAX_REQUEST_SIZE 16284
void PiServer::receiveThenHandleClientRequest(SOCKET clientSocket, const string& clientAddressString)
{
	char bufferRcv[MAX_REQUEST_SIZE];
	const auto recvLen = recv(clientSocket, bufferRcv, MAX_REQUEST_SIZE - 2, 0);

	if (recvLen > 0)
	{
		bufferRcv[recvLen] = '\0';

		if (printEverything)
		{
			cout << "[Received from " << clientAddressString << "]:\n" << bufferRcv << endl;
		}
	}

	const string data(bufferRcv);
	this->piController.doEverything(data);
	closesocket2(clientSocket);
}