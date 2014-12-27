#include "WebPageApp.h"
#include "Folder.h"
#include "Page.h"
#include "Server.h"
#include "ContentHost.h"
#include <iostream>

using namespace std;

WebPageApp::WebPageApp(Server* server) :
	WebApp("web", server)
{
}

void WebPageApp::HandleRequest(const std::string& request, ContentHost* contentHost, int clientSocket)
{
	const Folder* const dir = contentHost->GetRootDirectory();

	const Page* const page = request == "/" ? dir->GetDefaultPage() : static_cast<Page*>(dir->GetPage(request));

	if (page != NULL)
	{
		if (page->GetPageType() == PageType::HTML)
		{
			const Page* const constructedPage = this->ConstructPage(page, dir);
			this->server->SendPage(constructedPage, clientSocket);

			delete constructedPage;
		}
		else
		{
			this->server->SendPage(page, clientSocket);
		}
	}
	else
	{
		this->server->SendPage(dir->GetError404Page(), clientSocket, 404);
	}
}

Page const* WebPageApp::ConstructPage(const Page* const page, const Folder* const dir) const
{
	string pageContent(page->GetContent());
	
	auto dollarSignPosition = pageContent.find('$', 0);

	while (dollarSignPosition != string::npos)
	{
		const auto foundPosition = dollarSignPosition;
		const auto delimeterPosition = pageContent.find("=", dollarSignPosition + 1);
		const string operation = pageContent.substr(dollarSignPosition + 2, delimeterPosition - dollarSignPosition - 2);
		const auto eraseEnd = pageContent.find("}", foundPosition);
		const auto quotePosition = pageContent.find("\"", delimeterPosition + 2);
		const string operationTarget = pageContent.substr(delimeterPosition + 2, quotePosition - delimeterPosition - 2);

		if (operation == "LOAD")
		{
			const string data = static_cast<Page*>(dir->GetPage(operationTarget))->GetContent();
			pageContent.insert(foundPosition + eraseEnd - foundPosition + 1, data);
		}
		
		pageContent.erase(foundPosition, eraseEnd - foundPosition + 1);
		dollarSignPosition = pageContent.find('$', delimeterPosition + 1);
	}

	return new const Page(page->GetFullPath(), page->GetName(), pageContent);
}
