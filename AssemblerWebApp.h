#pragma once
#include "WebApp.h"

class Folder;
class Page;

class AssemblerWebApp : public WebApp
{
private:
	const Folder* const rootDirectory;
	const Page* const assemblerPage;

public:
	AssemblerWebApp(Server* server, const Folder* const rootDirectory);
	virtual void HandleRequest(const std::string& request, SOCKET clientSocket) override;
};