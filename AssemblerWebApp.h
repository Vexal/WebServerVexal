#pragma once
#include "WebApp.h"
#include "ContentHost.h"

class WebPageApp;

class AssemblerWebApp : public WebApp
{
private:
	const WebPageApp* const serverWebPageApp;

public:
	AssemblerWebApp(Server* server, const WebPageApp* const serverWebPageApp);
	virtual void HandleRequest(const std::string& request, ContentHost* contentHost, Client* client, int clientSocket) override;
};