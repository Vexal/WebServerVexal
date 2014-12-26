#pragma once

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
#define INVALID_SOCKET 0
#define SOCKET_ERROR -1
#endif
#include <map>

typedef void(*WebSocketCallback)();

class Client;
class Page;
class Folder;
class ContentHost;
class WebApp;

class Server
{
private:
	int serverSocket;
	std::string port;
	std::map<std::string, Client*> clients;
	std::map<std::string, std::string> hostNames;
	std::map<std::string, ContentHost*> virtualServers;
	std::map<std::string, WebSocketCallback> webSocketMessageCallbacks;
	std::map<std::string, WebApp*> webApps;
	
public:
	Server(const std::string& config = "config.txt");
	bool InitializeServer();
	void Update();
	void handleClientRequest(Client* client, const std::string& request, int requestLength, int clientSocket);
	bool SendPage(Client* client, const Page* const page, int clientSocket, int statusCode = 200);
	~Server();

private:
	bool initializeWSA();
	bool initializeTCPSocket();
	bool listenSocket();
	bool checkForNewConnection();
	void initializeWebSocketConnection(Client* client, int clientSocket, const std::string& request);
	void maintainWebSocketConnection(Client* client, int clientSocket);
	void sendWebSocketMessageShort(Client* client, int clientSocket, const std::string& message);
	bool initializeWebContent(const std::string& rootDirectory);
	Client* clientExists(const std::string& address) const;
	void handleSwitch();

public:
	static std::string cleanAssemblyString(std::string s, bool plussesAreSpaces = true);
};

void closesocket2(int socket);
void PostError();