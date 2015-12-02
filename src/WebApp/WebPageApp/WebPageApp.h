#pragma once
#include <map>
#include "../../Page/ContentHost.h"
#include "../../Logger/Logger.h"
#include "../WebApp.h"

class Page;
class Folder;
class ContentHost;
class CommentsDAO;

class WebPageApp : public WebApp
{
private:
	std::map<std::string, ContentHost*> virtualServers;
	const CommentsDAO* const commentsDAO;
	mutable Logger log = Logger("WebPageApp");

public:
	static std::map<std::string, int> viewCounts;

public:
	WebPageApp(HttpServer* server);
	virtual void HandleRequest(SOCKET clientSocket, const HttpRequest& httpRequest) override;
	const Folder* const GetRootDirectory(const std::string& hostName) const { return this->virtualServers.at(hostName)->GetRootDirectory(); }

private:
	std::string loadPageComments(const std::string& threadKey, const std::string& requestTarget) const;
};