
#define _CRT_SECURE_NO_WARNINGS
#include "Server.h"
#include "Client.h"
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
Server::Server(const std::string& config) : 
	port("8890"),
	serverSocket(INVALID_SOCKET)
{
	using namespace std;
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

#define MAX_REQUEST_SIZE 16284
void ForkThread(Server* server, Client* newClient, sockaddr_in* add2, int clientSocket)
{
	using namespace std;
	char bufferRcv[MAX_REQUEST_SIZE];
	const int recvLen = recv(clientSocket, bufferRcv, MAX_REQUEST_SIZE, 0);
	if(printEverything)
		cout << "Accept successful from address: " << newClient->GetAddressString() << " with " << recvLen << " bytes." << endl;

	if (recvLen >= 0 && recvLen < MAX_REQUEST_SIZE)
	{
		bufferRcv[recvLen] = '\0';
		if (printEverything && recvLen < MAX_REQUEST_SIZE)
		{
			cout << endl << endl << endl << bufferRcv << endl << endl << endl;
		}

		server->handleClientRequest(newClient, bufferRcv, recvLen, clientSocket);
	}
	else
	{
		closesocket2(clientSocket);
	}
}

bool Server::checkForNewConnection()
{
	using namespace std;
	int clientSocket;
	clientSocket = INVALID_SOCKET;

	sockaddr clientAddress;
	socklen_t clientAddressLength = sizeof(clientAddress);
	fd_set readSet;
	FD_ZERO(&readSet);
	FD_SET(this->serverSocket, &readSet);
	timeval timeout;
	timeout.tv_sec = 0;  // Zero timeout (poll)
	timeout.tv_usec = 0;
	//if (select(this->serverSocket, &readSet, NULL, NULL, &timeout) == 1)
	{
		//std::cout << "Bef4 socket\n";
		clientSocket = accept(this->serverSocket, &clientAddress, &clientAddressLength);
		//std::cout << "Aft3r socket\n";
	}
	//else
	{
	//	return false;
	}
	//u_long iMode = 1;
	//ioctlsocket(this->serverSocket, FIONBIO, &iMode);
	//if (clientSocket == INVALID_SOCKET || clientSocket < 0) 
	if (clientSocket < 0)
	{
		//cout << "Accept failed with error: " << endl;
		//PostError();
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

	std::string clientAddressString(inet_ntoa(add2->sin_addr));
	Client* newClient = this->clientExists(clientAddressString);

	if(newClient == NULL) //First recent connection from client
	{
		newClient = new Client(clientSocket, clientAddress, clientAddressString);
		if(printEverything)
			std::cout << "Received connection from new client with address " << clientAddressString << std::endl;
		this->clients[clientAddressString] = newClient;
	}
	else //Client exists
	{
		if(printEverything)
			std::cout << "Received request from existing client at address " << clientAddressString << std::endl;
	}

	add2 = reinterpret_cast<sockaddr_in*>(&newClient->GetAddress());
	thread serverThread(&ForkThread, this, newClient, add2, clientSocket);
	serverThread.detach();
	
	return true;
}

Client* Server::clientExists(const std::string& address) const
{
	auto potentialClient =  this->clients.find(address);
	if(potentialClient != this->clients.end())
	{
		return potentialClient->second;
	}

	return NULL;
}

void Server::handleSwitch()
{

}

void Server::handleClientRequest(Client* client, const std::string& request, int requestLength, int clientSocket)
{
	using namespace std;
	size_t nextPos = request.find_first_of(' ');
	string command(request, 0, nextPos);
	if(printEverything)
		cout << "Client request: " << command;

	if(command == "GET")
	{
		size_t connectionPosition = request.find("Connection: ");
		string connectionString;
		if (connectionPosition != string::npos)
		{
			size_t connectionPositionEnd = request.find_first_of('\r', connectionPosition + 12);
			connectionString = request.substr(connectionPosition + 12, connectionPositionEnd - connectionPosition - 12);
		}

		if (connectionString == "Upgrade")
		{
			//The client is attempting to initialize a WebSocket protocol connection
			this->initializeWebSocketConnection(client, clientSocket, request);
			this->maintainWebSocketConnection(client, clientSocket);			
		}
		else
		{
			size_t nextPos2 = request.find_first_of(' ', nextPos + 1);
			string file(request, nextPos + 1, nextPos2 - nextPos - 1);
			if (printEverything)
				cout << " with parameter " << file << endl;
			ClientRequest clientRequest = { "", "", "", "", "", "", "" };

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

			ofstream logFile;
			logFile.open("Connection Logs/" + client->GetAddressString() + ".slog", std::ios_base::app);
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

			string hostName = this->hostNames[clientRequest.hostName];
			if (hostName == "")
			{
				hostName = "Content/";
			}

			
			auto potentialClient = this->virtualServers.find(hostName);
			if (potentialClient != this->virtualServers.end())
			{
				auto potentialWebApp = this->webApps.end();
				const int firstQuestionMarkInd = file.find("?");
				if (firstQuestionMarkInd >= 0)
				{
					const string appName = file.substr(1, firstQuestionMarkInd - 1);
					potentialWebApp = this->webApps.find(appName);
				}
				if (potentialWebApp != this->webApps.end())
				{
					potentialWebApp->second->HandleRequest(file, potentialClient->second, client, clientSocket);
				}
				else
				{
					this->webApps["web"]->HandleRequest(file, potentialClient->second, client, clientSocket);
				}				
			}
		}
	}
}

bool Server::SendPage(Client* client, const Page* const page, int clientSocket, int statusCode)
{
	using namespace std;
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
		size_t iSendResult2 = send(clientSocket, finalPage.c_str(), finalPage.size(), 0);

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

std::string Server::cleanAssemblyString(std::string s, bool plussesAreSpaces)
{
	using namespace std;

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