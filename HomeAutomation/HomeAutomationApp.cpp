#include <iostream>

#include "HomeAutomationApp.h"
#include "..\DataAccess\DbUserDAO.h"
#include "..\SerialHandler.h"
#include "..\HttpServer.h"
#include "..\Folder.h"
#include "..\Page.h"

using namespace std;

HomeAutomationApp::HomeAutomationApp(HttpServer* server, const Folder* const rootDirectory) :
	WebApp("homeautomation", server),
	rootDirectory(rootDirectory),
	garagePage(static_cast<const Page* const>(rootDirectory->GetPage("/Projects/Garage/GarageControlPage.html"))),
	userDAO(DbUserDAO::Create())
{
	cout << "Initializing Serial Connection..." << endl;
	if (InitializeSerialConnection())
	{
		cout << "Serial connection initialization successful." << endl;
	}
	else
	{
		cout << "ERROR: Serial initialization failed." << endl;
	}
}

void HomeAutomationApp::HandleRequest(SOCKET clientSocket, const HttpRequest& httpRequest)
{
	const string accountName = GetStringParameter(httpRequest.request, "accountname");
	const string password = GetStringParameter(httpRequest.request, "password");

	string errorText = "";

	const string commandName = GetStringParameter(httpRequest.request, "submit");
	cout << "Received command " << commandName << " from user " << accountName << endl;
	/*cout << "Attempting to create account " << accountName << endl;
	if (accountName.empty())
	{
		errorText += CreateError("No account name specified");
	}
	if (password.empty())
	{
		errorText += CreateError("No password specified");
	}*/
	cout << "Sending TOGGLE command to serial...";
	char buf[7] = "toggle";
	buf[6] = '\n';
	if (WriteData(buf, 7))
	{
		cout << "..Successful" << endl;
	}
	else
	{
		cout << "..FAILED" << endl;
	}

	this->server->SendPage(this->garagePage, clientSocket, 302, "/projects/Garage/GarageControlPage.html");
}
