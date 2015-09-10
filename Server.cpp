#include <fstream>
#include <iostream>
#include <atomic>
#include <thread>

#include "Server.h"
#include "HttpServer.h"

using namespace std;

extern bool printEverything;
extern bool printThreading;

atomic_int threadCount;
atomic_int maxThreadCount;

Server::Server(const string& config)
{
	ifstream configFile("config.txt");
	maxThreadCount.store(8);
	if (configFile.is_open())
	{
		configFile >> this->port;
		cout << "Loaded from config file port: " << this->port.c_str() << " (len " << strlen(this->port.c_str()) << ")" << endl;

		int parseState = 0;
		string nextRoot;
		while (configFile.good())
		{
			string nextElement;
			configFile >> nextElement;
			if (nextElement == "root:")
			{
				parseState = 1;
			}
			else if (parseState == 1)
			{
				nextRoot = nextElement;
				parseState = 2;
			}
			else if (parseState == 2)
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
	cout << "initailizing http server..." << endl;
	this->httpServer.InitializeServer();
	cout << "Initializing " << maxThreadCount << " worker threads..." << endl;
	this->initializeWorkerThreads();

	cout << endl << "Initialization successful." << endl;
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
				const bool isHttpRequest = HttpServer::GetHttpRequestType(bufferRcv) != HttpRequestTypes::INVALID;

				if (isHttpRequest)
				{
					this->httpServer.HandleClientRequest(bufferRcv, clientSocket, clientAddressString);
				}
				else
				{
					cout << "WARN:  Received unknown request type " << bufferRcv << endl;
				}
			}
			catch (const std::out_of_range& e)
			{
				cout << "Exception caught: Invalid request: " << bufferRcv << endl;
			}
		}
		else
			break;
	} while (keepAlive);

#ifdef _WIN32
	closesocket(clientSocket);
#else
	close(socket);
#endif
}
