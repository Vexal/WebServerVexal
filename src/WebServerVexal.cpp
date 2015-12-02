#include "WebServerVexal.h"
#include <iostream>
#include <thread>
#include <string>
#include <fstream>
#include <atomic>
#include "Server.h"
using namespace std;

bool printEverything = false;
bool printThreading = false;
bool printError = false;

extern atomic_int maxThreadCount;
int main(int argc, char* argv[])
{
	thread serverThread(&ServerThread);
	serverThread.detach();
	while(true)
	{
		string input;
		cin >> input;

		if(input.compare("exit") == 0)
		{
			exit(0);
		}
		else if(input.compare("reload") == 0)
		{

		}
		else if (input == "tc")
		{
			printThreading = !printThreading;
		}
		else if (input == "maxthread")
		{
			int newThreadCount;
			cin >> newThreadCount;
			maxThreadCount.store(newThreadCount);
		}
		else if(input.compare("print") == 0)
		{
			printEverything = !printEverything;
		}
		else if (input == "printerror")
		{
			printError = !printError;
		}
	}

	return 0;
}

void ServerThread()
{
	Server server("config.txt");
	server.InitializeServer();

	while(true)
	{
		server.Update();
	}
}