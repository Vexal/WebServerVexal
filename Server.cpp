
#define _CRT_SECURE_NO_WARNINGS
#include "Server.h"
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

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "winmm.lib")

extern bool printEverything;
using namespace std;

Server::Server(const string& config) : 
	port("8890"),
	serverSocket(INVALID_SOCKET)
{
	ifstream configFile("config.txt");
	if(configFile.is_open())
	{
		configFile >> this->port;
		cout << "Loaded from config file port: " << this->port.c_str() << " (len " << strlen(this->port.c_str()) << ")" <<  endl;

		int parseState = 0;
		string nextRoot;
		while(configFile.good())
		{
			string nextElement;
			configFile >> nextElement;
			if(nextElement == "root:")
			{
				parseState = 1;
			}
			else if(parseState == 1)
			{
				nextRoot = nextElement;
				parseState = 2;
			}
			else if(parseState == 2)
			{
				this->hostNames[nextElement] = nextRoot;
			}
		}

		configFile.close();
	}

	this->webApps["vim"] = new VimWebApp(this);
	this->webApps["web"] = new WebPageApp(this);
	this->webApps["compile"] = new AssemblerWebApp(this, static_cast<WebPageApp*>(this->webApps["web"]));
}

void Server::Update()
{
	this->checkForNewConnection();
}

bool Server::checkForNewConnection()
{
	int clientSocket;
	clientSocket = INVALID_SOCKET;

	sockaddr clientAddress;
	socklen_t clientAddressLength = sizeof(clientAddress);
	fd_set readSet;
	FD_ZERO(&readSet);
	FD_SET(this->serverSocket, &readSet);

	//block here until a connection is found.
	clientSocket = accept(this->serverSocket, &clientAddress, &clientAddressLength);

	if (clientSocket < 0)
	{
		closesocket2(clientSocket);
		return false;
	}
	else if (clientSocket == INVALID_SOCKET)
	{
		cout << "inv" << endl;
		return false;
	}

	sockaddr_in* add2;
	add2 = reinterpret_cast<sockaddr_in*>(&clientAddress);

	const string clientAddressString(inet_ntoa(add2->sin_addr));
	if (printEverything)
		cout << "Received connection from client with address " << clientAddressString << endl;
	
	//create a new thread to handle the client's request.
	thread serverThread(&ForkThread, this, clientSocket, clientAddressString);
	serverThread.detach();
	
	return true;
}

#define MAX_REQUEST_SIZE 16284
void ForkThread(Server* server, int clientSocket, const string& clientAddressString)
{
	char bufferRcv[MAX_REQUEST_SIZE];
	const auto recvLen = recv(clientSocket, bufferRcv, MAX_REQUEST_SIZE - 1, 0);
	if (printEverything)
		cout << "Accept successful from address: " << clientAddressString << " with " << recvLen << " bytes." << endl;

	if (recvLen > 0)
	{
		bufferRcv[recvLen] = '\0';
		if (printEverything)
		{
			cout << endl << endl << endl << bufferRcv << endl << endl << endl;
		}

		server->handleClientRequest(bufferRcv, clientSocket, clientAddressString);
	}
	else
	{
		closesocket2(clientSocket);
	}
}

void Server::handleClientRequest(const string& request, int clientSocket, const string& clientAddressString)
{
	const size_t nextPos = request.find_first_of(' ');
	const string command(request, 0, nextPos);

	if(command == "GET")
	{
		this->handleHTTPGetRequest(request, clientSocket, clientAddressString);
	}
}

void Server::handleHTTPGetRequest(const string& request, int clientSocket, const string& clientAddressString)
{
	const size_t connectionPosition = request.find("Connection: ");
	string connectionString;
	if (connectionPosition != string::npos)
	{
		const size_t connectionPositionEnd = request.find_first_of('\r', connectionPosition + 12);
		connectionString = request.substr(connectionPosition + 12, connectionPositionEnd - connectionPosition - 12);
	}

	if (connectionString == "Upgrade")
	{
		//The client is attempting to initialize a WebSocket protocol connection
		this->initializeWebSocketConnection(clientSocket, request);
		this->maintainWebSocketConnection(clientSocket);
	}
	else
	{
		ClientRequest clientRequest = { clientAddressString, "", "", "", "", "", "", "" };

		this->parseClientHeader(request, clientRequest);
		this->writeClientLog(clientRequest);

		auto potentialClient = this->virtualServers.find("Content/");
		if (potentialClient != this->virtualServers.end())
		{
			//separate app arguments from app name.
			const auto firstQuestionMarkInd = clientRequest.requestTarget.find("?");

			const string appName = firstQuestionMarkInd != string::npos ?
				clientRequest.requestTarget.substr(1, firstQuestionMarkInd - 1) :
				clientRequest.requestTarget;

			const auto potentialWebApp = this->webApps.find(appName);

			//invoke web app if it exists, else handle request as web page.
			if (potentialWebApp != this->webApps.end())
			{
				potentialWebApp->second->HandleRequest(clientRequest.requestTarget, potentialClient->second, clientSocket);
			}
			else
			{
				this->webApps["web"]->HandleRequest(clientRequest.requestTarget, potentialClient->second, clientSocket);
			}
		}
	}
}

bool Server::SendPage(const Page* const page, int clientSocket, int statusCode)
{
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
		case 404:
			status = "404 NOT FOUND";
			break;
		}

		const string l1 = "HTTP/1.1 " + status + "\r\n";
		const string l2 = "Content-Type: " + contentType + "/" + pageType + "; charset=utf-8\r\n";
		const string l3 = "Content-Length: " + std::to_string(page->GetContentLength()) + "\r\n";
		const string content = string(page->GetContent(), page->GetContentLength());

		const string finalPage = l1 + l2 + l3 + "\r\n" + content;
		const size_t iSendResult2 = send(clientSocket, finalPage.c_str(), finalPage.size(), 0);

		if (iSendResult2 == SOCKET_ERROR || iSendResult2 <0) 
		{
			std::cout << "Send failed with error: " << std::endl;
			PostError();
			closesocket2(clientSocket);
			return false;
		}
	}

	closesocket2(clientSocket);
	return true;
}

Server::~Server()
{
#ifdef _WIN32
	WSACleanup();
#endif
	//delete this->rootDirectory;
}

void Server::parseClientHeader(const string& request, ClientRequest& clientRequest) const
{
	size_t nextPos = request.find_first_of(' ');
	size_t nextPos2 = request.find_first_of(' ', nextPos + 1);
	string file(request, nextPos + 1, nextPos2 - nextPos - 1);
	size_t referPosition = request.find("Referer: ");
	if (referPosition != string::npos) // link referred
	{
		size_t nextPos3 = request.find_first_of('\r', referPosition + 9);
		string referString(request, referPosition + 9, nextPos3 - referPosition - 9);
		size_t colonPos = referString.find_first_of(':');
		size_t nextSlashPos = referString.find_first_of('/', colonPos + 3);
		string referDomain(referString, colonPos + 3, nextSlashPos - colonPos - 3);
		clientRequest.refererDomain = referDomain;
		clientRequest.referer = referString;
	}
	else
	{
		clientRequest.referer = "'";
	}

	size_t userAgentPosition = request.find("User-Agent: ");
	if (userAgentPosition != string::npos)
	{
		size_t nextPos3 = request.find_first_of('\r', userAgentPosition + 12);
		string userAgentString(request, userAgentPosition + 12, nextPos3 - userAgentPosition - 12);

		clientRequest.userAgent = userAgentString;
	}
	else
	{
		clientRequest.userAgent = "USER-AGENT UNKNOWN";
	}

	size_t hostNamePo = request.find("Host: ");
	if (hostNamePo != string::npos)
	{
		size_t nextPos3 = request.find_first_of('\r', hostNamePo + 6);
		string userAgentString(request, hostNamePo + 6, nextPos3 - hostNamePo - 6);

		clientRequest.hostName = userAgentString;
	}
	else
	{
		clientRequest.hostName = "'";
	}
	clientRequest.requestCommand = "GET";
	clientRequest.requestTarget = file;
	// Current date/time based on current system
	time_t now = time(0);

	// Convert now to tm struct for local timezone
	tm* localtm = localtime(&now);

	clientRequest.requestTime = asctime(localtm);
	if (printEverything)
		cout << "The local date and time is: " << clientRequest.requestTime << endl;

}

void Server::writeClientLog(const ClientRequest& clientRequest) const
{
	ofstream logFile;
	logFile.open("Connection Logs/" + clientRequest.clientAddressString + ".slog", ios_base::app);
	if (logFile.is_open())
	{
		logFile << "*BEG*" << endl;
		logFile << "T: " << clientRequest.requestTime;
		logFile << "Command: " << clientRequest.requestCommand << endl;
		logFile << "Target: " << clientRequest.requestTarget << endl;
		logFile << "Refer Domain: " << clientRequest.refererDomain << endl;
		logFile << "Referer: " << clientRequest.referer << endl;
		logFile << "Hostname: " << clientRequest.hostName << endl;
		logFile << "Agent: " << clientRequest.userAgent << endl;
		logFile << "*END*" << endl << endl;
		logFile.close();
	}
	else
	{
		cout << "Error writing to log file!" << endl;
	}
}

string Server::cleanAssemblyString(string s, bool plussesAreSpaces)
{
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
		else if (s[a] == '%')
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