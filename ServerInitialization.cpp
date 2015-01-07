#include "Server.h"
#include <iostream>
#include <string>
#include "ContentHost.h"


bool Server::InitializeServer()
{
	this->initializeWSA();
	std::cout << "Initializing Web Content... port: " << this->port << std::endl;
	this->initializeWebContent("");
	std::cout << "Initializing Server Socket..." << std::endl;
	this->initializeTCPSocket();
	std::cout << "Listening Server Socket..." << std::endl;
	this->listenSocket();
	return true;
}

bool Server::initializeWebContent(const std::string& rootDirectory)
{
	//this->rootDirectory = new Folder("Content/", "Content");
	this->virtualServers["Content/"] = new ContentHost("Content");
	this->virtualServers["OtherContent/"] = new ContentHost("OtherContent");
	return true;
}

bool Server::initializeWSA()
{
#ifdef _WIN32
	WSADATA wsaData;
	int iResult;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		std::cout << "WSA initialization failed: " << iResult << std::endl;
		PostError();
		return false;
	}

	std::cout << "WSA initialization successful." << std::endl;
#endif
	return true;
}

bool Server::initializeTCPSocket()
{
	addrinfo *result = NULL, *ptr = NULL, hints;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the local address and port to be used by the server
	auto iResult = getaddrinfo(NULL, this->port.c_str(), &hints, &result);
	if (iResult != 0)
	{
		std::cout << "getaddrinfo failed: " << iResult << std::endl;
		PostError();
#ifdef _WIN32
		WSACleanup();
#endif
		return false;
	}

	this->serverSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

	if (this->serverSocket == INVALID_SOCKET)
	{
		std::cout << "Error at socket(): " << std::endl;
		PostError();
		freeaddrinfo(result);
#ifdef _WIN32
		WSACleanup();
#endif
		return false;
	}

	std::cout << "Socket creation successful." << std::endl;

	auto iResult2 = bind(this->serverSocket, result->ai_addr, result->ai_addrlen);

	if (iResult2 == SOCKET_ERROR)
	{
		std::cout << "bind() failed with error: " << std::endl;
		PostError();
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

bool Server::listenSocket()
{
	auto res = listen(this->serverSocket, SOMAXCONN);

	//u_long iMode = 1;
	//ioctlsocket(this->serverSocket, FIONBIO, &iMode);
	if (res == SOCKET_ERROR)
	{
		std::cout << "Listen failed with error: " << std::endl;
		PostError();
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

void closesocket2(int socket)
{
	//std::cout << "closing socket " << socket << std::endl;
#ifdef _WIN32
	closesocket(socket);
#else
	close(socket);
#endif
}

void PostError()
{
	std::cout << "ERROR" << std::endl;
}