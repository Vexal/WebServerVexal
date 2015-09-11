
#define _CRT_SECURE_NO_WARNINGS
#include "HttpServer.h"
#include <stdio.h>
#include <iostream>
#include "Page.h"
#include "Folder.h"
#include <thread>
#include <ctime>
#include <fstream>
#include "ContentHost.h"
#include <sstream>
#include "AssemblerWebApp.h"
#include "VimWebApp.h"
#include "WebPageApp.h"

#ifdef _WIN32
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "winmm.lib")
#else
#include <time.h>
#endif
#include "AccountCreateApp.h"
#include "HttpServer.h"


extern bool printEverything;
extern bool printThreading;

using namespace std;

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
	this->webApps["vim"] = new VimWebApp(this);
	this->webApps["compile"] = new AssemblerWebApp(this, static_cast<WebPageApp*>(this->webApps["web"])->GetRootDirectory("Content/"));
	this->webApps["createaccount"] = new AccountCreateApp(this, static_cast<WebPageApp*>(this->webApps["web"])->GetRootDirectory("Content/"));
	return true;
}

void HttpServer::HandleClientRequest(const string& request, SOCKET clientSocket, const string& clientAddressString) const
{
	
	const HttpRequestTypes requestType = HttpServer::GetHttpRequestType(request);
	switch (requestType)
	{
	case HttpRequestTypes::GET:
		this->handleHTTPGetRequest(request, clientSocket, clientAddressString);
		break;
	case HttpRequestTypes::POST:
		this->handleHTTPGetRequest(request, clientSocket, clientAddressString);
		break;
	default:
		return;
	}
}

void HttpServer::handleHTTPGetRequest(const string& request, SOCKET clientSocket, const string& clientAddressString) const
{
	const ConnectionType connectionType = HttpServer::GetConnectionType(request);
	if (connectionType == ConnectionType::UPGRADE)
	{
		//The client is attempting to initialize a WebSocket protocol connection
		//this->initializeWebSocketConnection(clientSocket, request);
		//this->maintainWebSocketConnection(clientSocket);
		return;
	}

	ClientRequest clientRequest = { clientAddressString, "", "", "", "", "", "", "", "", "" };

	this->parseClientHeader(request, clientRequest);
	const unordered_map<string, string> paramMap = HttpServer::GetHttpGetParameters(request);
	const auto potentialWebApp = this->webApps.find(clientRequest.requestTarget);

	//invoke web app if it exists, else handle request as web page.
	if (potentialWebApp != this->webApps.end())
	{
		potentialWebApp->second->HandleRequest(clientSocket, { clientRequest.fullRequest, paramMap });
	}
	else
	{
		this->webApps.at("web")->HandleRequest(clientSocket, { clientRequest.fullRequest, paramMap });
	}

	this->writeClientLog(clientRequest);
}

bool HttpServer::SendPage(const Page* const page, SOCKET clientSocket, int statusCode, const string& redirectUrl) const
{
	//need to move this function to another class, or perhaps the WebpageApp class.
	string contentType = "text";
	string pageType = "html";
	string charSet = "charset=utf-8\r\n";
	string status = "200 OK";

	if(page != NULL)
	{
		char* recvbuf = page->GetContent();
		char headerType = 0;
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
		const string l2 = "Content-Type: " + contentType + "/" + pageType + "; charset=utf-8\r\n";
		const string l3 = "Content-Length: " + to_string(page->GetContentLength()) + "\r\n";
		const string content = string(page->GetContent(), page->GetContentLength());

		const string finalPage = l1 + l2 + l3 + "\r\n" + content;
		
		const auto sendAmount = send(clientSocket, finalPage.c_str(), finalPage.size(), 0);

		//cout << "Sending to socket " << clientSocket << endl;
		if (sendAmount == SOCKET_ERROR || sendAmount < 0)
		{
			cout << "Send failed with error: " << endl;
#ifdef _WIN32
			closesocket(clientSocket);
#else
			close(socket);
#endif
			return false;
		}
	}

	return true;
}

HttpServer::~HttpServer()
{
#ifdef _WIN32
	WSACleanup();
#endif
}

void HttpServer::parseClientHeader(const string& request, ClientRequest& clientRequest) const
{
	const size_t nextPos = request.find_first_of(' ');
	if (nextPos == string::npos) return;
	const size_t nextPos2 = request.find_first_of(' ', nextPos + 1);
	if (nextPos2 == string::npos) return;
	const string file(request, nextPos + 1, nextPos2 - nextPos - 1);
	const size_t referPosition = request.find("Referer: ");

	const auto firstQuestionMarkInd = (file.length() > 4 && file.substr(0, 4) == "/vim") ? 4 : file.find("?"); //need to refactor this line.

	if (firstQuestionMarkInd != string::npos)
	{
		clientRequest.requestArguments = file.substr(firstQuestionMarkInd + 1);
		clientRequest.requestTarget = file.substr(1, firstQuestionMarkInd - 1);
	}
	else
	{
		clientRequest.requestTarget = file;
	}

	clientRequest.fullRequest = file;

	if (referPosition != string::npos) // link referred
	{
		const size_t nextPos3 = request.find_first_of('\r', referPosition + 9);
		if (nextPos3 == string::npos) return;
		const string referString(request, referPosition + 9, nextPos3 - referPosition - 9);
		const size_t colonPos = referString.find_first_of(':');
		if (colonPos == string::npos) return;
		const size_t nextSlashPos = referString.find_first_of('/', colonPos + 3);
		if (nextSlashPos == string::npos) return;
		const string referDomain(referString, colonPos + 3, nextSlashPos - colonPos - 3);
		clientRequest.refererDomain = referDomain;
		clientRequest.referer = referString;
	}
	else
	{
		clientRequest.referer = "'";
	}

	const size_t userAgentPosition = request.find("User-Agent: ");
	if (userAgentPosition != string::npos)
	{
		const size_t nextPos3 = request.find_first_of('\r', userAgentPosition + 12);
		if (nextPos3 == string::npos) return;
		const string userAgentString(request, userAgentPosition + 12, nextPos3 - userAgentPosition - 12);

		clientRequest.userAgent = userAgentString;
	}
	else
	{
		clientRequest.userAgent = "USER-AGENT UNKNOWN";
	}

	const size_t hostNamePo = request.find("Host: ");
	if (hostNamePo != string::npos)
	{
		const size_t nextPos3 = request.find_first_of('\r', hostNamePo + 6);
		if (nextPos3 == request.npos) return;
		const string userAgentString(request, hostNamePo + 6, nextPos3 - hostNamePo - 6);

		clientRequest.hostName = userAgentString;
	}
	else
	{
		clientRequest.hostName = "'";
	}
	clientRequest.requestCommand = "GET";

	clientRequest.requestTime = "";
}

void HttpServer::writeClientLog(const ClientRequest& clientRequest) const
{
	logMutex.lock();

	ofstream logFile;
	logFile.open("Connection Logs/" + clientRequest.clientAddressString + ".slog", ios_base::app);
	if (logFile.is_open())
	{
		const time_t now = time(0);
		const tm* const localtm = localtime(&now);

		logFile << "*BEG*" << endl;
		logFile << "T: " << asctime(localtm);
		logFile << "Command: " << clientRequest.requestCommand << endl;
		logFile << "Target: " << clientRequest.fullRequest << endl;
		logFile << "Refer Domain: " << clientRequest.refererDomain << endl;
		logFile << "Referer: " << clientRequest.referer << endl;
		logFile << "Hostname: " << clientRequest.hostName << endl;
		logFile << "Agent: " << clientRequest.userAgent << endl;
		logFile << "*END*" << endl << endl;
		logFile.close();
	}

	logMutex.unlock();
}

string HttpServer::cleanAssemblyString(string s, bool plussesAreSpaces)
{
	//need to move this function somewhere more appropriate.
	string newString;
	newString.reserve(s.length());

	for (int a = 0; a < s.length(); ++a)
	{
		if (s[a] == 0)
		{
			break;
		}
		if (s[a] == '+' && plussesAreSpaces)
		{
			newString.append(" ");
		}
		else if (s[a] == '%' && a + 2 < s.length())
		{
			if (s[a + 2] == 'A' && s[a + 1] == '0')
			{
				newString.append({ 10 });
			}
			else if (s[a + 2] == 'A' && s[a + 1] == '3')
			{
				newString.append(":");
			}
			else if (s[a + 2] == 'F' && s[a + 1] == '2')
			{
				newString.append("/");
			}
			else if (s[a + 2] == '0' && s[a + 1] == '2')
			{
				newString.append(" ");
			}
			a += 2;
		}
		else
		{
			newString.append({ s[a] });
		}
	}

	return newString;
}

HttpRequestTypes HttpServer::GetHttpRequestType(const string& request)
{
	const string firstWord = request.substr(0, request.find_first_of(' '));
	return GetHttpRequestTypeFromString(firstWord);
}

unordered_map<string, string> HttpServer::GetHttpGetParameters(const string& getRequest)
{
	const size_t requestStart = 4;
	//const size_t requestSize = getR
	const string fullGetString = getRequest.substr(4, getRequest.find_first_of(' ', 4) - 4);
	unordered_map<string, string> paramMap;
	size_t currentInd = fullGetString.find_first_of('?');

	while (currentInd != string::npos)
	{
		++currentInd;
		if (currentInd > fullGetString.length())
			break;

		const size_t nextInd = fullGetString.find_first_of('&', currentInd);

		const string paramAndValue = fullGetString.substr(currentInd, nextInd - currentInd);
		const size_t equals = paramAndValue.find_first_of('=');
		const string param = paramAndValue.substr(0, equals);
		const string value = paramAndValue.substr(equals + 1);

		if (!param.empty() && !value.empty())
		{
			paramMap[param] = value;
		}

		currentInd = nextInd;
	}

	return paramMap;
}

HttpRequestTypes HttpServer::GetHttpRequestTypeFromString(const string& requestWord)
{
	if (requestWord == "GET")
	{
		return HttpRequestTypes::GET;
	}
	if (requestWord == "POST")
	{
		return HttpRequestTypes::POST;
	}

	return HttpRequestTypes::INVALID;
}

ConnectionType HttpServer::GetConnectionType(const string& request)
{
	const size_t connectionPosition = request.find("Connection: ");
	if (connectionPosition != string::npos)
	{
		const size_t connectionPositionEnd = request.find_first_of('\r', connectionPosition + 12);
		if (connectionPositionEnd == string::npos) return ConnectionType::NONE;
		const string connectionString = request.substr(connectionPosition + 12, connectionPositionEnd - connectionPosition - 12);
		if (connectionString == "Upgrade")
		{
			return ConnectionType::UPGRADE;
		}
	}

	return ConnectionType::NONE;
}