#include <fstream>
#include <iostream>
#include <atomic>
#include <thread>

#include "Server.h"
#include "HttpServer/HttpServer.h"
#include "HttpServer/HttpUtils.h"

using namespace std;

extern bool printEverything;
extern bool printThreading;

atomic_int threadCount;
atomic_int maxThreadCount;

Logger Server::log = Logger("Server");

Server::Server(const string& config)
{
	maxThreadCount.store(8);
	this->loadConfig(config);
}

bool Server::InitializeServer()
{
	this->initializeWSA();
	cout << "Initializing Server Socket..." << endl;
	this->initializeTCPSocket();
	cout << "Listening Server Socket..." << endl;
	this->listenSocket();
	cout << "Initializing http server..." << endl;
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
	this->workQueueCondition.notify_all();

	return true;
}

void Server::workerThreadHandler(Server* server)
{
	while (true)
	{
		unique_lock<mutex> workLock(server->workQueueMutex);
		server->workQueueCondition.wait(workLock, [server]() {return !server->workerDataQueue.empty();});

		const WorkerData clientData = server->workerDataQueue.front();
		server->workerDataQueue.pop();
		workLock.unlock();

		++threadCount; //for debugging, inconsequential.
		if (printThreading)
			cout << "Thread count: " << threadCount << endl;

		server->receiveThenHandleClientRequest(clientData.clientSocket, clientData.clientAddressString, clientData.keepAlive);
		--threadCount;
	}
}

#define MAX_REQUEST_SIZE 16284
void Server::receiveThenHandleClientRequest(SOCKET clientSocket, const string& clientAddressString, bool keepAlive) const
{
	do
	{
		char bufferRcv[MAX_REQUEST_SIZE];
		const auto recvLen = recv(clientSocket, bufferRcv, MAX_REQUEST_SIZE - 2, 0);
		if (recvLen == SOCKET_ERROR)
		{
			log.error("Error recv from ip " + clientAddressString);
		}
		log.info("Accept successful from socket " + to_string(clientSocket) + " with " + to_string(recvLen) + " bytes from ip " + clientAddressString);

		if (recvLen > 0)
		{
			bufferRcv[recvLen] = '\0';
			log.info(bufferRcv);
			if (printEverything)
			{
				cout << "[Received from " << clientAddressString << "]:\n" << bufferRcv << endl;
			}

			try
			{
				const bool handledHttpRequest = this->httpServer.HandleClientRequest(bufferRcv, clientSocket, clientAddressString);

				if (!handledHttpRequest)
				{
					log.error("Failed to handle request: " + string(bufferRcv) + " from " + clientAddressString);
				}
			}
			catch (const std::out_of_range& e)
			{
				log.error("Exception caught: Invalid request: Out of range exception " + string(bufferRcv) + " from " + clientAddressString);
			}
			catch (const std::length_error& e)
			{
				log.error("Exception caught: Invalid request: length error exception " + string(bufferRcv) + " from " + clientAddressString);
			}
		}
		else
			break;
	} while (keepAlive);

#ifdef _WIN32
	const auto ret = shutdown(clientSocket, SD_SEND);
#else
	const auto ret = shutdown(clientSocket, SHUT_WR);
#endif
	if (ret == SOCKET_ERROR)
	{
		closesocket2(clientSocket);
		return;
	}
	char shutdownBuffer[MAX_REQUEST_SIZE];
	long long shutdownRecvLen = -2;
	do
	{
		shutdownRecvLen = recv(clientSocket, shutdownBuffer, MAX_REQUEST_SIZE - 2, 0);
		if (shutdownRecvLen == SOCKET_ERROR)
		{
			log.error("failed to recv shutdown recv from " + clientAddressString);
		}
	} while (shutdownRecvLen > 0);

	closesocket2(clientSocket);
}
