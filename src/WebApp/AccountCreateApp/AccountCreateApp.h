#pragma once
#include "../WebApp.h"

class Folder;
class Page;
class UserDAO;

class AccountCreateApp : public WebApp
{
private:
	const Folder* const rootDirectory;
	const Page* const accountPage;
	const Page* const accountSuccessfulPage;
	UserDAO* const userDAO;

public:
	AccountCreateApp(HttpServer* server, const Folder* const rootDirectory);
	virtual void HandleRequest(SOCKET clientSocket, const HttpRequest& httpRequest) override;
};