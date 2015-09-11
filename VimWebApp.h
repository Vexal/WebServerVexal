#pragma once
#include "WebApp.h"

class VimWebApp : public WebApp
{
public:
	VimWebApp(HttpServer* server);
	virtual void HandleRequest(SOCKET clientSocket, const HttpRequest& httpRequest) override;
};