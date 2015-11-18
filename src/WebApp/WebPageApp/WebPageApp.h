#pragma once
#include "../WebApp.h"
#include "../../Page/ContentHost.h"
#include <map>

class Page;
class Folder;
class ContentHost;
class CommentsDAO;

class WebPageApp : public WebApp
{
private:
	std::map<std::string, ContentHost*> virtualServers;
	const CommentsDAO* const commentsDAO;

public:
	static std::map<std::string, int> viewCounts;

public:
	WebPageApp(HttpServer* server);
	virtual void HandleRequest(SOCKET clientSocket, const HttpRequest& httpRequest) override;
	const Folder* const GetRootDirectory(const std::string& hostName) const { return this->virtualServers.at(hostName)->GetRootDirectory(); }
};