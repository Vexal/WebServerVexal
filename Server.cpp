
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
#include <atomic>

#ifdef _WIN32
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "winmm.lib")
#else
#include <time.h>
#endif
#include "AccountCreateApp.h"


extern bool printEverything;
extern bool printThreading;

using namespace std;

atomic_int threadCount;
atomic_int maxThreadCount;

Server::Server(const string& config)
{
	ifstream configFile("config.txt");
	maxThreadCount.store(8);
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
}

bool Server::InitializeServer()
{
	this->initializeWSA();
	cout << "Initializing Server Socket..." << endl;
	this->initializeTCPSocket();
	cout << "Listening Server Socket..." << endl;
	this->listenSocket();
	cout << "Initializing Web Content... port: " << this->port << endl;
	this->initializeWebContent("");
	cout << "Initializing " << maxThreadCount << " worker threads..." << endl;
	this->initializeWorkerThreads();

	cout << endl << "Initialization successful." << endl;
	return true;
}

bool Server::initializeWebContent(const string& rootDirectory)
{
	this->webApps["web"] = new WebPageApp(this);
	this->webApps["vim"] = new VimWebApp(this);
	this->webApps["compile"] = new AssemblerWebApp(this, static_cast<WebPageApp*>(this->webApps["web"])->GetRootDirectory("Content/"));
	this->webApps["createaccount"] = new AccountCreateApp(this, static_cast<WebPageApp*>(this->webApps["web"])->GetRootDirectory("Content/"));
	return true;
}

void Server::initializeWorkerThreads()
{
	for (int a = 0; a < maxThreadCount; ++a)
	{
		thread workerThread = thread(&workerThreadHandler, this);
		workerThread.detach();
	}
}

void Server::Update()
{
	this->checkForNewConnection();
}

bool Server::checkForNewConnection()
{
	sockaddr clientAddress;
	socklen_t clientAddressLength = sizeof(clientAddress);
	fd_set readSet;
	FD_ZERO(&readSet);
	FD_SET(this->serverSocket, &readSet);

	//block here until a connection is found.
	const auto clientSocket = accept(this->serverSocket, &clientAddress, &clientAddressLength);

	if (clientSocket < 0 || clientSocket == INVALID_SOCKET)
	{
		cout << "inv" << endl;
		closesocket2(clientSocket);
		return false;
	}

	const sockaddr_in* const add2 = reinterpret_cast<sockaddr_in*>(&clientAddress);

	const string clientAddressString(inet_ntoa(add2->sin_addr));
	if (printEverything)
		cout << "Received connection from client with address " << clientAddressString << endl;

	//pass the client data to the worker thread queue.
	this->workQueueMutex.lock();
	this->workerDataQueue.push({ clientSocket, clientAddressString, false });
	this->workQueueMutex.unlock();

	return true;
}

void Server::workerThreadHandler(Server* server)
{
	while (true)
	{
		server->workQueueMutex.lock();
		if (!server->workerDataQueue.empty())
		{
			const WorkerData clientData = server->workerDataQueue.front();
			server->workerDataQueue.pop();
			server->workQueueMutex.unlock();

			++threadCount; //for debugging, inconsequential.
			if (printThreading)
				cout << "Thread count: " << threadCount << endl;

			server->receiveThenHandleClientRequest(clientData.clientSocket, clientData.clientAddressString, clientData.keepAlive);
			--threadCount;
		}
		else
		{
			server->workQueueMutex.unlock();
#ifdef _WIN32
			Sleep(4);
#else //linux
			timespec t = { 0, 16000000 };
			nanosleep(&t, nullptr);
#endif
		}
	}
}

#define MAX_REQUEST_SIZE 16284
void Server::receiveThenHandleClientRequest(SOCKET clientSocket, const string& clientAddressString, bool keepAlive) const
{
	do 
	{
		char bufferRcv[MAX_REQUEST_SIZE];
		const auto recvLen = recv(clientSocket, bufferRcv, MAX_REQUEST_SIZE - 1, 0);
		if (printEverything)
			cout << "Accept successful from address: " << clientSocket << " with " << recvLen << " bytes." << endl;

		if (recvLen > 0)
		{
			bufferRcv[recvLen] = '\0';
			if (printEverything)
			{
				cout << endl << endl << endl << bufferRcv << endl << endl << endl;
			}

			try
			{
				this->handleClientRequest(bufferRcv, clientSocket, clientAddressString);
			}
			catch (const std::out_of_range& e)
			{
				cout << "Exception caught: Invalid request: " << bufferRcv << endl;
			}
		}
		else
			break;
	} while (keepAlive);

	closesocket2(clientSocket);
}

void Server::handleClientRequest(const string& request, SOCKET clientSocket, const string& clientAddressString) const
{
	const size_t nextPos = request.find_first_of(' ');
	const string command(request, 0, nextPos);

	if(command == "GET" || command == "POST")
	{
		this->handleHTTPGetRequest(request, clientSocket, clientAddressString);
	}
}

void Server::handleHTTPGetRequest(const string& request, SOCKET clientSocket, const string& clientAddressString) const
{
	const size_t connectionPosition = request.find("Connection: ");
	if (connectionPosition != string::npos)
	{
		const size_t connectionPositionEnd = request.find_first_of('\r', connectionPosition + 12);
		const string connectionString = request.substr(connectionPosition + 12, connectionPositionEnd - connectionPosition - 12);
		if (connectionString == "Upgrade")
		{
			//The client is attempting to initialize a WebSocket protocol connection
			this->initializeWebSocketConnection(clientSocket, request);
			this->maintainWebSocketConnection(clientSocket);
			return;
		}
	}

	ClientRequest clientRequest = { clientAddressString, "", "", "", "", "", "", "", "", "" };

	this->parseClientHeader(request, clientRequest);

	const auto potentialWebApp = this->webApps.find(clientRequest.requestTarget);

	//invoke web app if it exists, else handle request as web page.
	if (potentialWebApp != this->webApps.end())
	{
		potentialWebApp->second->HandleRequest(clientRequest.fullRequest, clientSocket);
	}
	else
	{
		this->webApps.at("web")->HandleRequest(clientRequest.requestTarget, clientSocket);
	}

	this->writeClientLog(clientRequest);
}

bool Server::SendPage(const Page* const page, SOCKET clientSocket, int statusCode, const string& redirectUrl) const
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
			PostError();
			closesocket2(clientSocket);
			return false;
		}
	}

	return true;
}

Server::~Server()
{
#ifdef _WIN32
	WSACleanup();
#endif
}

void Server::parseClientHeader(const string& request, ClientRequest& clientRequest) const
{
	const size_t nextPos = request.find_first_of(' ');
	const size_t nextPos2 = request.find_first_of(' ', nextPos + 1);
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
		const string referString(request, referPosition + 9, nextPos3 - referPosition - 9);
		const size_t colonPos = referString.find_first_of(':');
		const size_t nextSlashPos = referString.find_first_of('/', colonPos + 3);
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

void Server::writeClientLog(const ClientRequest& clientRequest) const
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

string Server::cleanAssemblyString(string s, bool plussesAreSpaces)
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