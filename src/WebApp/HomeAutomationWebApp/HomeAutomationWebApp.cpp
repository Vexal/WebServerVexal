#include <iostream>

#include "HomeAutomationWebApp.h"
#include "../../DataAccess/MySql/DbUserDAO.h"
#include "../../DataAccess/DataErrorException.h"
#include "../../Serial/SerialController.h"
#include "../../HttpServer/HttpServer.h"
#include "../../HttpServer/HttpRequest.h"
#include "../../Page/Folder.h"
#include "../../Page/Page.h"
#include "../../HomeAutomation/HomeAutomationController.h"

using namespace std;

HomeAutomationWebApp::HomeAutomationWebApp(HttpServer* server, const Folder* const rootDirectory) :
	WebApp("homeautomation", server),
	rootDirectory(rootDirectory),
	garagePage(static_cast<const Page* const>(rootDirectory->GetPage("/Projects/Garage/GarageControlPage.html"))),
	authenticationResponsePage(static_cast<const Page* const>(rootDirectory->GetPage("/Projects/Garage/AuthenticationResponse.html"))),
	userDAO(DbUserDAO::Create()),
	controller(HomeAutomationController::Create())
{
	
}

void HomeAutomationWebApp::HandleRequest(SOCKET clientSocket, const HttpRequest& httpRequest)
{
	const string accountName = httpRequest.GetParameter("accountName");
	const string password = httpRequest.GetParameter("password");

	string errorText = "";

	const string commandName = httpRequest.GetParameter("submit");
	log.info("Received command " + commandName + " from user " + accountName);
	cout << "Received command " << commandName << " from user " << accountName << endl;
	if (accountName.empty())
	{
		this->server->SendPage(this->authenticationResponsePage, clientSocket, 302, "/projects/Garage/AuthenticationResponse.html");
		return;
	}
	if (password.empty())
	{
		this->server->SendPage(this->authenticationResponsePage, clientSocket, 302, "/projects/Garage/AuthenticationResponse.html");
		return;
	}

	unordered_set<string> accessTypes;
	try
	{
		const int userId = this->userDAO->GetUserId(accountName, password);
		accessTypes = this->userDAO->GetAccessTypes(userId);
	}
	catch (const DataErrorException& e)
	{
		this->log.error("Data error: " + e.error);
	}
	catch (const InvalidCredentialsException& e)
	{
		this->log.error("Invalid credentials from user " + accountName);
	}

	if (accessTypes.find("garage") == accessTypes.end())
	{
		this->log.error("Illegal garage access attempt from user " + accountName);
		this->server->SendPage(this->authenticationResponsePage, clientSocket, 302, "/projects/Garage/AuthenticationResponse.html");
		return;
	}

	string resultStatusMessage;
	const int resultErrorCode = this->controller->ActivateGarage(resultStatusMessage);

	if (resultErrorCode == 0)
	{
		this->log.info("Controller successfully activated garage by user " + accountName);
		this->server->SendPage(this->garagePage, clientSocket, 302, "/projects/Garage/GarageControlPage.html");
	}
	else
	{
		this->log.error("Controller failed to activate garage by user " + accountName);
		this->server->SendPage(this->authenticationResponsePage, clientSocket, 302, "/projects/Garage/AuthenticationResponse.html");
	}

}
