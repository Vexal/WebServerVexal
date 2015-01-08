#pragma once
#include "WebApp.h"
#include "ContentHost.h"
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
	WebPageApp(Server* server);
	virtual void HandleRequest(const std::string& request, SOCKET clientSocket) override;
	const Folder* const GetRootDirectory(const std::string& hostName) const { return this->virtualServers.at(hostName)->GetRootDirectory(); }

public:
	static Page const* ConstructPage(const Page* const page, const Folder* const root);
};