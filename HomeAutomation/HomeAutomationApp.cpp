#include <iostream>

#include "HomeAutomationApp.h"
#include "../DataAccess/DbUserDAO.h"
#include "../SerialHandler.h"
#include "../HttpServer.h"
#include "../Folder.h"
#include "../Page.h"
#include "../DataAccess/DataErrorException.h"

using namespace std;

HomeAutomationApp::HomeAutomationApp(HttpServer* server, const Folder* const rootDirectory) :
	WebApp("homeautomation", server),
	rootDirectory(rootDirectory),
	garagePage(static_cast<const Page* const>(rootDirectory->GetPage("/Projects/Garage/GarageControlPage.html"))),
	authenticationResponsePage(static_cast<const Page* const>(rootDirectory->GetPage("/Projects/Garage/AuthenticationResponse.html"))),
	userDAO(DbUserDAO::Create())
{
	/*cout << "Initializing Serial Connection..." << endl;
	if (InitializeSerialConnection())
	{
		cout << "Serial connection initialization successful." << endl;
	}
	else
	{
		cout << "ERROR: Serial initialization failed." << endl;
	}*/
}

void HomeAutomationApp::HandleRequest(SOCKET clientSocket, const HttpRequest& httpRequest)
{
	const string accountName = GetStringParameter(httpRequest.request, "accountname");
	const string password = GetStringParameter(httpRequest.request, "password");

	string errorText = "";

	const string commandName = GetStringParameter(httpRequest.request, "submit");
	cout << "Received command " << commandName << " from user " << accountName << endl;
	if (accountName.empty())
	{
		cout << "..FAILED" << endl;
		this->server->SendPage(this->authenticationResponsePage, clientSocket, 302, "/projects/Garage/AuthenticationResponse.html");
		return;
	}
	if (password.empty())
	{
		cout << "..FAILED" << endl;
		this->server->SendPage(this->authenticationResponsePage, clientSocket, 302, "/projects/Garage/AuthenticationResponse.html");
		return;
	}

	unordered_set<string> accessTypes;
	try
	{
		const int userId = this->userDAO->GetUserId(accountName, password);
		if (userId <= 0)
		{
			cout << "Invalid Password Authentication" << endl;
			this->server->SendPage(this->authenticationResponsePage, clientSocket, 302, "/projects/Garage/AuthenticationResponse.html");
			return;
		}

		accessTypes = this->userDAO->GetAccessTypes(userId);
	}
	catch (const DataErrorException& e)
	{
		cout << "data error: " << e.error << endl;
	}
	catch (const InvalidCredentialsException& e)
	{
		cout << "Invalid Credentials" << endl;
	}

	if (accessTypes.find("garage") == accessTypes.end())
	{
		cout << "..FAILED AUTHENTICATION NO GARAGE ACCESS" << endl;
		this->server->SendPage(this->authenticationResponsePage, clientSocket, 302, "/projects/Garage/AuthenticationResponse.html");
		return;
	}


	cout << "Sending TOGGLE command to serial...";
	char buf[7] = "toggle";
	buf[6] = '\n';
	if (WriteData(buf, 7))
	{
		cout << "..Successful" << endl;
		this->server->SendPage(this->garagePage, clientSocket, 302, "/projects/Garage/GarageControlPage.html");
	}
	else
	{
		cout << "..FAILED" << endl;
		this->server->SendPage(this->authenticationResponsePage, clientSocket, 302, "/projects/Garage/AuthenticationResponse.html");
	}

}
