#include <iostream>
#include <fstream>
#include <algorithm>
#include "WebPageApp.h"
#include "../../Page/Folder.h"
#include "../../Page/Page.h"
#include "../../HttpServer/HttpServer.h"
#include "../../HttpServer/HttpRequest.h"
#include "../../Page/PageConstructor.h"
#include "../../DataAccess/Null/NullCommentsDAO.h"

using namespace std;

map<string, int> WebPageApp::viewCounts;

WebPageApp::WebPageApp(HttpServer* server) :
	WebApp("web", server),
	commentsDAO(NullCommentsDAO::Create())
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
			unordered_map<string, string> params;
			params["comments"] = this->commentsDAO->GetThread(5).GenerateFullHtml();
			const Page* const constructedPage = PageConstructor::ConstructPage(page, dir, params);
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
