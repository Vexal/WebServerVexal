#pragma once
#include "WebApp.h"

class VimWebApp : public WebApp
{
public:
	VimWebApp(Server* server);
	virtual void HandleRequest(const std::string& request, SOCKET clientSocket) override;
};