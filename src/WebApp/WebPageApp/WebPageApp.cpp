#include <iostream>
#include <fstream>
#include <algorithm>
#include "WebPageApp.h"
#include "../../Page/Folder.h"
#include "../../Page/Page.h"
#include "../../HttpServer/HttpServer.h"
#include "../../HttpServer/HttpRequest.h"
#include "../../Page/PageConstructor.h"
#include "../../DataAccess/MySql/DbCommentsDAO.h"
#include "../../DataAccess/DataErrorException.h"
#include "../../CommentsSystem/CommentsException.h"
#include "../../Captcha/CaptchaGenerator.h"

using namespace std;

map<string, int> WebPageApp::viewCounts;

WebPageApp::WebPageApp(HttpServer* server) :
	WebApp("web", server),
	commentsDAO(DbCommentsDAO::Create()),
	captchaGenerator(CaptchaGenerator::Create())
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

	const Page* const page = httpRequest.requestTarget == "/" ? dir->GetDefaultPage() : static_cast<Page*>(dir->GetPage(httpRequest.requestTarget));

	if (page != nullptr)
	{
		if (page->GetPageType() == PageType::HTML)
		{
			unordered_map<string, string> params;
			unordered_map<string, function<string(string const&)> > paramFuncs;
			const string threadKey = CommentsDAO::GetThreadKey(httpRequest.requestTarget);

			paramFuncs["comments"] = [&threadKey, &httpRequest, this](const string& op) {
				return this->loadPageComments(threadKey, httpRequest.requestTarget);
			};

			paramFuncs["captchadata"] = [this](const string& op) {
				return this->captchaGenerator->GenerateCaptcha().GenerateHtmlFormInput();
			};

			params["requestTarget"] = httpRequest.requestTarget;
			params["threadKey"] = threadKey;
			const Page* const constructedPage = PageConstructor::ConstructPage(page, dir, httpRequest, params, paramFuncs);
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

string WebPageApp::loadPageComments(const string& threadKey, const string& requestTarget) const
{
	try
	{
		return this->commentsDAO->GetThread(threadKey).GenerateFullHtml();
	}
	catch (const DataErrorException& e)
	{
		this->log.error("Data exception loading comments for " + requestTarget + " (key: " + threadKey + "): " + e.error);
	}
	catch (const CommentsException& e)
	{
		this->log.error("Comments exception loading comments for " + requestTarget + " (key: " + threadKey + "): " + e.error);
	}

	return "";
}
