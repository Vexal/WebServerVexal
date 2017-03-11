#pragma once
//deprecated//deprecated//deprecated//deprecated//deprecated//deprecated//deprecated//deprecated
#include "../WebApp.h"
//deprecated
class Folder;
class Page;
class UserDAO;

//deprecated
//deprecated
class AccountCreateApp : public WebApp
{
private:
	//deprecated
	const Folder* const rootDirectory;
	//deprecated
	const Page* const accountPage;
	//deprecated
	const Page* const accountSuccessfulPage;
	//deprecated
	UserDAO* const userDAO;

public:
	//deprecated
	AccountCreateApp(HttpServer* server, const Folder* const rootDirectory);//deprecated
	virtual void HandleRequest(SOCKET clientSocket, const HttpRequest& httpRequest) override;//deprecated//deprecated//deprecated
};
//deprecated
//deprecated
//deprecated