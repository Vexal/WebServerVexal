#pragma once
#include "../WebApp.h"

class Folder;
class Page;
class UserDAO;

class HomeAutomationApp : public WebApp
{
private:
	const Folder* const rootDirectory;
	const Page* const garagePage;
	const Page* const authenticationResponsePage;
	UserDAO* const userDAO;

public:
	HomeAutomationApp(HttpServer* server, const Folder* const rootDirectory);
	virtual void HandleRequest(SOCKET clientSocket, const HttpRequest& httpRequest) override;
};