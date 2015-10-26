#include <iostream>

#include "AccountCreateApp.h"
#include "../WebPageApp/WebPageApp.h"
#include "../../HttpServer/HttpServer.h"
#include "../../Page/Folder.h"
#include "../../Page/Page.h"
#include "../../DataAccess/MySql/DbUserDAO.h"
#include "../../DataAccess/DataErrorException.h"
#include "../../HttpServer/HttpRequest.h"

using namespace std;

AccountCreateApp::AccountCreateApp(HttpServer* server, const Folder* const rootDirectory) :
	WebApp("createaccount", server),
	rootDirectory(rootDirectory),
	accountPage(static_cast<const Page* const>(rootDirectory->GetPage("/Projects/MMORTS/MMORTS.html"))),
	accountSuccessfulPage((static_cast<const Page* const>(rootDirectory->GetPage("/Projects/MMORTS/Account.html")))),
	userDAO(DbUserDAO::Create())
{
}

string CreateError(const string& message)
{
	return "<span class='accerr'>" + message + "</span>\n";
}

void AccountCreateApp::HandleRequest(SOCKET clientSocket, const HttpRequest& httpRequest)
{
	const string accountName = GetStringParameter(httpRequest.request, "accountname");
	const string password = GetStringParameter(httpRequest.request, "password");

	string errorText = "";

	cout << "Attempting to create account " << accountName << endl;
	if (accountName.empty())
	{
		errorText += CreateError("No account name specified");
	}
	if (password.empty())
	{
		errorText += CreateError("No password specified");
	}

	if (errorText.empty())
	{
		try
		{
			this->userDAO->CreateAccount(accountName, password);
		}
		catch (const DuplicateAccountException& e)
		{
			errorText += CreateError("Error: username " + accountName + " already exists.");
		}
		catch (const DataErrorException& e)
		{
			errorText += CreateError("Data Error: " + e.error);
		}
	}

	vector<string> replaceTokens;
	vector<string> replaceTexts;

	if (errorText.empty())
	{
		cout << "Account creation for account " << accountName << " successful." << endl;
		replaceTexts.push_back("Successfully created account " + accountName + "\n");
		replaceTokens.push_back("Account: ");
		const Page* const newPage = this->accountSuccessfulPage->ClonePage(replaceTokens, replaceTexts);
		const Page* const constructedPage = WebPageApp::ConstructPage(newPage, rootDirectory);
		this->server->SendPage(constructedPage, clientSocket, 302, "/projects/mmorts/Account.html");
	}
	else
	{
		cout << "Account creation for account " << accountName << " failed." << endl;
		cout << errorText << endl;

		replaceTokens.push_back("Create result: ");
		replaceTexts.push_back(errorText);
		const Page* const newPage = accountPage->ClonePage(replaceTokens, replaceTexts);
		const Page* const constructedPage = WebPageApp::ConstructPage(newPage, rootDirectory);
		this->server->SendPage(constructedPage, clientSocket, 200);
		delete constructedPage;

		delete newPage;
	}	
}