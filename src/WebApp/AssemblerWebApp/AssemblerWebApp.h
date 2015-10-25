#pragma once
#include "../WebApp.h"

class Folder;
class Page;

class AssemblerWebApp : public WebApp
{
private:
	const Folder* const rootDirectory;
	const Page* const assemblerPage;

public:
	AssemblerWebApp(HttpServer* server, const Folder* const rootDirectory);
	virtual void HandleRequest(SOCKET clientSocket, const HttpRequest& httpRequest) override;
};