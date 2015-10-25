#pragma once
#include <string>
#include <unordered_map>
#ifdef _WIN32
#include <winsock2.h>
#else
#define SOCKET int
#endif

#include "HttpRequest.h"

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

	static std::string GetStringParameter(const std::string& input, const std::string& parameterName); // return first instance of param
};
