#include <iostream>

#include "CommentWebApp.h"
#include "../../DataAccess/MySql/DbUserDAO.h"
#include "../../DataAccess/MySql/DbCommentsDAO.h"
#include "../../DataAccess/DataErrorException.h"
#include "../../HttpServer/HttpServer.h"
#include "../../HttpServer/HttpRequest.h"
#include "../../Page/Folder.h"
#include "../../Page/Page.h"

using namespace std;

CommentWebApp::CommentWebApp(HttpServer* server, const Folder* const rootDirectory) :
	WebApp("commentpost", server),
	rootDirectory(rootDirectory),
	userDAO(DbUserDAO::Create()),
	commentsDAO(DbCommentsDAO::Create())
{

}

void CommentWebApp::HandleRequest(SOCKET clientSocket, const HttpRequest& httpRequest)
{
	const string accountName = httpRequest.GetParameter("accountname");
	const string password = httpRequest.GetParameter("password");
	const string threadKey = httpRequest.GetParameterDecoded("threadkey");

	string errorText = "";
	string infoText = "";
	const string commentText = httpRequest.GetParameterDecoded("commenttext");
	log.info("Received comment " + commentText + " from user " + accountName);
	cout << "Received comment " << commentText << " from user " << accountName << endl;
	bool validInfo = true;
	if (accountName.empty())
	{
		validInfo = false;
		errorText += "Account name missing. ";
	}
	if (password.empty())
	{
		validInfo = false;
		errorText += "Password missing. ";
	}
	if (commentText.empty())
	{
		validInfo = false;
		errorText += "Comment text missing. ";
	}

	unordered_set<string> accessTypes;
	if (validInfo)
	{
		try
		{
			bool wasCreated = false;
			const User user = this->userDAO->GetValidatedAccount(accountName, password, true, wasCreated);
			cout << "Got user " << user.id << " " << user.username << endl;
			accessTypes = this->userDAO->GetAccessTypes(user.id);

			if (wasCreated)
			{
				infoText += "Created account " + accountName;
			}

			this->commentsDAO->PostComment(threadKey, user, httpRequest.clientAddressString, commentText);
		}
		catch (const DataErrorException& e)
		{
			this->log.error("Data error: " + e.error);
			errorText += "Database error. ";
		}
		catch (const InvalidCredentialsException& e)
		{
			this->log.error("Invalid credentials from user " + accountName);
			errorText += "Invalid credentials for existing account. ";
		}
	}

	string responseUrl = httpRequest.GetParameterDecoded("source");

	if (!errorText.empty())
	{
		responseUrl += "?error=";
		responseUrl += errorText;
	}
	
	if (!infoText.empty())
	{
		responseUrl += errorText.empty() ? "?info=" : "&info=";
		responseUrl += infoText;
	}

	this->server->SendPage(nullptr, clientSocket, 302, responseUrl);
}
