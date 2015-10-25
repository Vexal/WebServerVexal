#pragma once
#include "../WebApp.h"
#include "../../Page/ContentHost.h"
#include <map>

class Page;
class Folder;
class ContentHost;

class WebPageApp : public WebApp
{
private:
	static std::map<std::string, int> viewCounts;
	std::map<std::string, ContentHost*> virtualServers;

public:
	WebPageApp(HttpServer* server);
	virtual void HandleRequest(SOCKET clientSocket, const HttpRequest& httpRequest) override;
	const Folder* const GetRootDirectory(const std::string& hostName) const { return this->virtualServers.at(hostName)->GetRootDirectory(); }

public:
	static Page const* ConstructPage(const Page* const page, const Folder* const root);
};