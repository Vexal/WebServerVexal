#pragma once
#include "../WebApp.h"
#include "../../Logger/Logger.h"

class Folder;
class Page;
class UserDAO;
class HomeAutomationController;

class HomeAutomationWebApp : public WebApp
{
private:
	const Folder* const rootDirectory;
	UserDAO* const userDAO;
	HomeAutomationController* const controller;
	Logger log = Logger("HomeAutomationWebApp");

public:
	HomeAutomationWebApp(HttpServer* server, const Folder* const rootDirectory);
	virtual void HandleRequest(SOCKET clientSocket, const HttpRequest& httpRequest) override;
};