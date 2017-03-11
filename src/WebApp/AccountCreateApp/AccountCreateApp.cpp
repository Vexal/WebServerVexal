#include <iostream>
//deprecated
#include "AccountCreateApp.h"
#include "../../HttpServer/HttpServer.h"
#include "../../Page/Folder.h"
#include "../../Page/Page.h"
#include "../../DataAccess/MySql/DbUserDAO.h"
#include "../../DataAccess/DataErrorException.h"
#include "../../HttpServer/HttpRequest.h"
#include "../../Page/PageConstructor.h"

using namespace std;
//deprecated
AccountCreateApp::AccountCreateApp(HttpServer* server, const Folder* const rootDirectory) :
	WebApp("createaccount", server),
	rootDirectory(rootDirectory),
	accountPage(nullptr),
	accountSuccessfulPage(nullptr),
	userDAO(DbUserDAO::Create())
{
}

//deprecated
void AccountCreateApp::HandleRequest(SOCKET clientSocket, const HttpRequest& httpRequest)
{
		
}
//deprecated
//deprecated
//deprecated
//deprecated
//deprecated
//deprecated