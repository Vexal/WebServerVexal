#pragma once
#include <string>
#ifdef _WIN32
#include <winsock2.h>
#else
#define SOCKET int
#endif
#include "../HttpServer/HttpRequest.h"


class HttpServer;
class Client;

class WebApp
{
protected:
	const std::string name;
	HttpServer* const server;

public:
	WebApp(const std::string& name, HttpServer* server);
	virtual void HandleRequest(SOCKET clientSocket, const HttpRequest& httpRequest) = 0;
	virtual ~WebApp();

	std::string GetName() const { return this->name; }
};
