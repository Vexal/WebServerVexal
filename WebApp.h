#pragma once
#include <string>

class Server;
class ContentHost;
class Client;

class WebApp
{
protected:
	const std::string name;
	Server* const server;

public:
	WebApp(const std::string& name, Server* server);
	virtual void HandleRequest(const std::string& request, ContentHost* contentHost, Client* client, int clientSocket) = 0;
	virtual ~WebApp();

	std::string GetName() const { return this->name; }

};
