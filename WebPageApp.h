#pragma once
#include "WebApp.h"
#include <map>

class Page;
class Folder;

class WebPageApp : public WebApp
{
private:
	mutable std::map<std::string, int> viewCounts;

public:
	WebPageApp(Server* server);
	virtual void HandleRequest(const std::string& request, ContentHost* contentHost, int clientSocket) override;

	Page const* ConstructPage(const Page* const page, const Folder* const root) const;

private:
};