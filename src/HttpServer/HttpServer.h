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
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define SOCKET int
#endif
#include <map>
#include <mutex>
#include <queue>
#include <unordered_map>
#include "HttpRequest.h"
#include "../Logger/Logger.h"

typedef void(*WebSocketCallback)();

class Page;
class Folder;
class ContentHost;
class WebApp;
class SerialController;

class HttpServer
{
private:
	std::unordered_map<std::string, WebSocketCallback> webSocketMessageCallbacks;
	std::unordered_map<std::string, WebApp*> webApps;
	SerialController* const serialController;
	mutable Logger log = Logger("HttpServer");

public:
	HttpServer();
	bool InitializeServer();
	bool SendPage(const Page* const page, SOCKET clientSocket, int statusCode = 200, const std::string& redirectUrl = "") const;
	~HttpServer();

	bool HandleClientRequest(const std::string& request, SOCKET clientSocket, const std::string& clientAddressString) const;

private:
	bool handleHTTPRequest(const std::string& request, SOCKET clientSocket, const std::string& clientAddressString) const;
	void initializeWebSocketConnection(int clientSocket, const std::string& request) const;
	void maintainWebSocketConnection(int clientSocket) const;
	void sendWebSocketMessageShort(int clientSocket, const std::string& message) const;
	bool initializeWebContent(const std::string& rootDirectory);
	void writeClientLogEntry(const HttpRequest& request) const;
};
