#pragma once
#include "../WebApp.h"

class Folder;
class Page;
class UserDAO;
class HomeAutomationController;

class HomeAutomationWebApp : public WebApp
{
private:
	const Folder* const rootDirectory;
	const Page* const garagePage;
	const Page* const authenticationResponsePage;
	UserDAO* const userDAO;
	HomeAutomationController* const controller;

public:
	HomeAutomationWebApp(HttpServer* server, const Folder* const rootDirectory);
	virtual void HandleRequest(SOCKET clientSocket, const HttpRequest& httpRequest) override;
};