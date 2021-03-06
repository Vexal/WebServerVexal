#pragma once
#include <string>
#include <map>
#include <queue>
#include <mutex>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

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
#include <string>


#include <netdb.h>

#include <unistd.h>
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define SOCKET int
#endif

#include "HttpServer/HttpServer.h"
#include "Logger/Logger.h"

struct WorkerData
{
	SOCKET clientSocket;
	std::string clientAddressString;
	bool keepAlive;
};

//This is lowest level of the server, responsible for handling sockets and assigning connections to threads.
class Server
{
	SOCKET serverSocket = INVALID_SOCKET;
	std::string port = "8890";
	HttpServer httpServer;
	std::map<std::string, std::string> hostNames;
	mutable std::mutex workQueueMutex;
	std::queue<WorkerData> workerDataQueue;
	std::condition_variable workQueueCondition;
	static Logger log;

public:
	Server(const std::string& config = "config.txt");
	bool InitializeServer();

	void Update();

private:
	bool initializeWSA();
	bool initializeTCPSocket();
	void initializeWorkerThreads();
	bool listenSocket();

	bool checkForNewConnection();
	void receiveThenHandleClientRequest(SOCKET clientSocket, const std::string& clientAddressString, bool keepAlive) const;
	void loadConfig(const std::string& config);
private:
	static void workerThreadHandler(Server* server);
};

void closesocket2(SOCKET);