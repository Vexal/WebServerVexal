#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <iostream>
#include <thread>
#include <ctime>
#include <fstream>
#include <sstream>
#include "../Util.h"
#include "../Page/ContentHost.h"
#include "../Page/Folder.h"
#include "../Page/Page.h"
#include "../WebApp/AssemblerWebApp/AssemblerWebApp.h"
#include "../WebApp/VimWebApp/VimWebApp.h"
#include "../WebApp/WebPageApp/WebPageApp.h"
#include "../WebApp/AccountCreateApp/AccountCreateApp.h"
#include "../WebApp/HomeAutomationWebApp/HomeAutomationWebApp.h"
#include "../WebApp/CommentWebApp/CommentWebApp.h"
#include "../Serial/SerialController.h"
#include "HttpServer.h"
#include "HttpUtils.h"
#include "HttpRequest.h"

#ifdef _WIN32
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "winmm.lib")
#else
#include <time.h>
#endif

using namespace std;

extern bool printEverything;
extern bool printThreading;

HttpServer::HttpServer() :
	serialController(SerialController::Create())
{

}

bool HttpServer::InitializeServer()
{
	cout << "Initializing Web Content... port: " << endl;
	this->initializeWebContent("");

	cout << endl << "Initialization successful." << endl;
	return true;
}

bool HttpServer::initializeWebContent(const string& rootDirectory)
{
	this->webApps["web"] = new WebPageApp(this);
	//this->webApps["/vim"] = new VimWebApp(this);
	this->webApps["/compile"] = new AssemblerWebApp(this, static_cast<WebPageApp*>(this->webApps["web"])->GetRootDirectory("Content/"));
	this->webApps["/homeautomation"] = new HomeAutomationWebApp(this, static_cast<WebPageApp*>(this->webApps["web"])->GetRootDirectory("Content/"));
	this->webApps["/commentpost"] = new CommentWebApp(this, static_cast<WebPageApp*>(this->webApps["web"])->GetRootDirectory("Content/"));
	return true;
}

bool HttpServer::HandleClientRequest(const string& request, SOCKET clientSocket, const string& clientAddressString) const
{	
	return this->handleHTTPRequest(request, clientSocket, clientAddressString);
}

bool HttpServer::handleHTTPRequest(const string& request, SOCKET clientSocket, const string& clientAddressString) const
{
	//split into headers and body
	const vector<string> requestComponents = Util::split(request, "\r\n\r\n");

	if (requestComponents.empty())
	{
		this->log.error("Received empty request from socket " + to_string(clientSocket) + " from ip " + clientAddressString);
		return false;
	}

	const unordered_map<string, string> headers = HttpUtils::GetHttpHeaders(requestComponents[0] + "\r\n");

	if (Util::get_with_default<string, string>(headers, "connection", "") == "Upgrade")
	{
		//The client is attempting to initialize a WebSocket protocol connection
		//this->initializeWebSocketConnection(clientSocket, request);
		//this->maintainWebSocketConnection(clientSocket);
		return true;
	}

	const string requestMethod = Util::get_with_default<string, string>(headers, HttpUtils::REQUEST_METHOD_FIELD, "");
	if (requestMethod != "GET" && requestMethod != "POST")
	{
		this->log.error("Strange request method: " + requestMethod + " from ip " + clientAddressString + "\n\n" + Util::PrintMap(headers));
		return false;
	}

	const string requestUri = Util::get_with_default<string, string>(headers, HttpUtils::REQUEST_URI_FIELD, "");
	//split uri into target and parameters
	const vector<string> uriComponents = Util::split(requestUri, "?");
	if (uriComponents.empty())
	{
		this->log.error("Empty uri components " + requestUri + " from ip " + clientAddressString);
		return false;
	}

	const HttpRequest httpRequest = { requestUri,
		uriComponents.size() > 1 ? HttpUtils::GetHttpGetParameters(uriComponents[1]) : unordered_map<string, string>(),
		headers,
		uriComponents[0],
		HttpUtils::GetDomain(Util::get_with_default<string, string>(headers, "referer", "")),
		clientAddressString
	};

	const auto potentialWebApp = this->webApps.find(httpRequest.requestTarget);

	//invoke web app if it exists, else handle request as web page.
	if (potentialWebApp != this->webApps.end())
	{
		potentialWebApp->second->HandleRequest(clientSocket, httpRequest);
	}
	else
	{
		this->webApps.at("web")->HandleRequest(clientSocket, httpRequest);
	}

	this->writeClientLogEntry(httpRequest);
	return true;
}

bool HttpServer::SendPage(const Page* const page, SOCKET clientSocket, int statusCode, const string& redirectUrl) const
{
	string contentType = "text";
	string pageType = "html";
	string charSet = "charset=utf-8\r\n";
	string status = "200 OK";

	if (page != nullptr)
	{
		switch (page->GetContentType())
		{
		case ContentType::TEXT:
			contentType = "text";
			break;
		case ContentType::IMAGE:
			contentType = "image";
			break;
		}
		switch (page->GetPageType())
		{
		case PageType::HTML:
			pageType = "html";
			break;
		case PageType::CSS:
			pageType = "css";
			break;
		case PageType::GIF:
			pageType = "gif";
			break;
		case PageType::ICO:
			pageType = "x-icon";
			break;
		case PageType::JPG:
			pageType = "jpeg";
			break;
		case PageType::PNG:
			pageType = "png";
			break;
		default:
			break;
		}
	}

	switch (statusCode)
	{
	case 200:
		status = "200 OK";
		break;
	case 302:
		status = string("302 Found\r\n") + string("Location: ") + redirectUrl;
		break;
	case 404:
		status = "404 NOT FOUND";
		break;
	}

	const string l1 = "HTTP/1.1 " + status + "\r\n";

	string finalPage = l1;

	if (page != nullptr)
	{
		const string l2 = "Content-Type: " + contentType + "/" + pageType + "; charset=utf-8\r\n";
		const string l3 = "Content-Length: " + to_string(page->GetContentLength()) + "\r\n";
		const string content = string(page->GetContent(), page->GetContentLength());

		finalPage += l2;
		finalPage += l3;
		finalPage += "\r\n";
		finalPage += content;
	}
	else
	{
		finalPage += "\r\n";
	}
		
	const auto sendAmount = send(clientSocket, finalPage.c_str(), finalPage.size(), 0);
	if (sendAmount == SOCKET_ERROR || sendAmount < 0)
	{
		cout << "Send failed with error: " << endl;
#ifdef _WIN32
		closesocket(clientSocket);
#else
		close(clientSocket);
#endif
		return false;
	}
	
	return true;
}

HttpServer::~HttpServer()
{
#ifdef _WIN32
	WSACleanup();
#endif
}

void HttpServer::writeClientLogEntry(const HttpRequest& clientRequest) const
{
	const string fileName = "Connection Logs/" + clientRequest.clientAddressString + ".slog";
	string logEntry;
	logEntry.reserve(1000);
	logEntry += "*BEG*\n";
	logEntry += "T: "; logEntry += Util::CurrentDateTime(); logEntry += "\n";
	if (!clientRequest.referDomain.empty())
	{
		logEntry += "Refer Domain: "; logEntry += clientRequest.referDomain; logEntry += "\n";
	}
	if (!clientRequest.requestTarget.empty())
	{
		logEntry += "Request Target: "; logEntry += clientRequest.requestTarget; logEntry += "\n";
	}
	for (const auto& pair : clientRequest.headers)
	{
		logEntry += pair.first; logEntry += ": "; logEntry += pair.second; logEntry += "\n";
	}

	logEntry += "*END*\n";
	this->log.append(fileName, logEntry);
}
