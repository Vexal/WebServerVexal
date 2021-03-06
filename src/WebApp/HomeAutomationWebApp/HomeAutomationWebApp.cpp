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
	userDAO(DbUserDAO::Create()),
	controller(HomeAutomationController::Create())
{
	
}

void HomeAutomationWebApp::HandleRequest(SOCKET clientSocket, const HttpRequest& httpRequest)
{
	const string accountName = httpRequest.GetParameter("accountname");
	const string password = httpRequest.GetParameter("password");
	const string activationObject = httpRequest.GetParameter("activateWhat");

	string errorText = "";

	const string commandName = httpRequest.GetParameter("submit");
	log.info("Received command " + commandName + " from user " + accountName);
	cout << "Received command " << commandName << " from user " << accountName << endl;
	
	if (activationObject == "garage")
	{
		if (accountName.empty())
		{
			this->server->SendPage(nullptr, clientSocket, 302, "/projects/Garage/AuthenticationResponse.html");
			return;
		}
		if (password.empty())
		{
			this->server->SendPage(nullptr, clientSocket, 302, "/projects/Garage/AuthenticationResponse.html");
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
			this->server->SendPage(nullptr, clientSocket, 302, "/projects/Garage/AuthenticationResponse.html");
			return;
		}

		string resultStatusMessage;
		//const int resultErrorCode = this->controller->ActivateGarage(resultStatusMessage);
		const int resultErrorCode = this->controller->ActivateGarageNetwork(resultStatusMessage);

		if (resultErrorCode == 0)
		{
			this->log.info("Controller successfully activated garage by user " + accountName);
			this->server->SendPage(nullptr, clientSocket, 302, "/projects/Garage/GarageControlPage.html");
		}
		else
		{
			this->log.error("Controller failed to activate garage by user " + accountName);
			this->server->SendPage(nullptr, clientSocket, 302, "/projects/Garage/AuthenticationResponse.html");
		}
	}
	else if (activationObject == "roomAlarm")
	{
		string resultStatusMessage;
		//const int resultErrorCode = this->controller->ActivateGarage(resultStatusMessage);
		const int resultErrorCode = this->controller->ActivateRoomAlarmNetwork(resultStatusMessage);

		if (resultErrorCode == 0)
		{
			this->log.info("Controller successfully activated garage by user " + accountName);
			this->server->SendPage(nullptr, clientSocket, 302, "/projects/Alarm/Alarm.html");
		}
		else
		{
			this->log.error("Controller failed to activate garage by user " + accountName);
			this->server->SendPage(nullptr, clientSocket, 302, "/projects/Alarm/AuthenticationResponse.html");
		}
	}
}
