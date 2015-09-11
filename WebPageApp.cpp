#include "WebPageApp.h"
#include "Folder.h"
#include "Page.h"
#include "HttpServer.h"
#include <iostream>
#include <fstream>
#include <algorithm>

using namespace std;

map<string, int> WebPageApp::viewCounts;

WebPageApp::WebPageApp(HttpServer* server) :
	WebApp("web", server)
{
	ifstream inputFile("viewcounts.txt");
	
	if (inputFile.is_open())
	{
		while (inputFile.good())
		{
			string tokenName;
			int tokenValue;

			inputFile >> tokenName;
			inputFile >> tokenValue;

			WebPageApp::viewCounts["content" + tokenName] = tokenValue;
		}

		inputFile.close();
	}

	this->virtualServers["Content/"] = new ContentHost("Content");
	this->virtualServers["OtherContent/"] = new ContentHost("OtherContent");
}

void WebPageApp::HandleRequest(SOCKET clientSocket, const HttpRequest& httpRequest)
{
	const Folder* const dir = this->virtualServers.at("Content/")->GetRootDirectory();

	const Page* const page = httpRequest.request == "/" ? dir->GetDefaultPage() : static_cast<Page*>(dir->GetPage(httpRequest.request));

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

Page const* WebPageApp::ConstructPage(const Page* const page, const Folder* const dir)
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
		else if (operation == "THIS")
		{
			string pa = page->GetFullPath();
			transform(pa.begin(), pa.end(), pa.begin(), ::tolower);
			const auto vc = WebPageApp::viewCounts.find(pa);
			const int viewCount = vc != WebPageApp::viewCounts.end() ? vc->second : 0;
			pageContent.insert(foundPosition + eraseEnd - foundPosition + 1, to_string(viewCount));
		}
		
		pageContent.erase(foundPosition, eraseEnd - foundPosition + 1);
		dollarSignPosition = pageContent.find('$', delimeterPosition + 1);
	}

	return new const Page(page->GetFullPath(), page->GetName(), pageContent);
}
