#pragma once
#include "WebApp.h"

class Page;
class Folder;

class WebPageApp : public WebApp
{
public:
	WebPageApp(Server* server);
	virtual void HandleRequest(const std::string& request, ContentHost* contentHost, int clientSocket) override;

	Page const* ConstructPage(const Page* const page, const Folder* const root) const;

private:
};