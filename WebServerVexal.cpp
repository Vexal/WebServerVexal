// WebServerVexal.cpp : Defines the entry point for the console application.
//

#include "WebServerVexal.h"
#include "Server.h"
#include <iostream>
#include <thread>
#include <string>
#include <fstream>
#include "SerialHandler.h"
using namespace std;

bool resetServer = true;
bool printEverything = false;

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
			resetServer = true;
		}
		else if(input.compare("print") == 0)
		{
			printEverything = !printEverything;
		}
		else if (input.compare("initserial") == 0)
		{
			cout << "Initializing Arduino serial connection..";
			if (InitializeSerialConnection())
			{
				cout << "..Successful" << endl;
			}
			else
			{
				cout << "..FAILED" << endl;
			}
		}
	}

	return 0;
}

void ServerThread()
{
	Server* server = NULL;
	while(true)
	{
		if(resetServer)
		{
			if(server != NULL)
			{
				delete server;
			}
				
			server = new Server("config.txt");
			

			server->InitializeServer();

			resetServer = false;
		};
		
		if(server != NULL)
		{
			server->Update();
		}
	}
}