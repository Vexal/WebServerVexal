#include <iostream>

#include "../../Captcha/CaptchaGenerator.h"
#include "../../DataAccess/MySql/DbUserDAO.h"
#include "../../DataAccess/MySql/DbCommentsDAO.h"
#include "../../DataAccess/DataErrorException.h"
#include "../../HttpServer/HttpServer.h"
#include "../../HttpServer/HttpRequest.h"
#include "../../Page/Folder.h"
#include "../../Page/Page.h"
#include "CommentWebApp.h"

using namespace std;

CommentWebApp::CommentWebApp(HttpServer* server, const Folder* const rootDirectory) :
	WebApp("commentpost", server),
	rootDirectory(rootDirectory),
	userDAO(DbUserDAO::Create()),
	commentsDAO(DbCommentsDAO::Create()),
	captchaGenerator(CaptchaGenerator::Create())
{

}

void CommentWebApp::HandleRequest(SOCKET clientSocket, const HttpRequest& httpRequest)
{
	const string accountName = httpRequest.GetParameter("accountname");
	const string password = httpRequest.GetParameter("password");
	const string threadKey = httpRequest.GetParameterDecoded("threadkey");
	const string commentText = httpRequest.GetParameterDecoded("commenttext");
	const string captchaAnswer = httpRequest.GetParameter("captchaanswer");
	const string captchaTime = httpRequest.GetParameter("captchatime");
	const string captchaHash = httpRequest.GetParameter("captchahash");

	string errorText = "";
	string infoText = "";
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
	if (captchaAnswer.empty() || captchaHash.empty() || captchaTime.empty())
	{
		validInfo = false;
		errorText += "Captcha data missing.";
	}

	if (validInfo)
	{
		const bool validCaptcha = this->captchaGenerator->ValidateAnswer(captchaHash, captchaAnswer, captchaTime);
		if (validCaptcha)
		{
			try
			{
				bool wasCreated = false;
				const User user = this->userDAO->GetValidatedAccount(accountName, password, true, wasCreated);

				if (wasCreated)
				{
					infoText += "Created account " + accountName;
				}

				this->commentsDAO->PostComment(threadKey, user, httpRequest.clientAddressString, commentText);

				infoText += " Comment successfully posted. ";
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
		else
		{
			errorText += "Invalid captcha answer. ";
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
