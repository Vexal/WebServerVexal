#include "VimWebApp.h"
#include "HttpServer.h"
#include "Page.h"

using namespace std;

VimWebApp::VimWebApp(HttpServer* server) :
	WebApp("vim", server)
{
}

void VimWebApp::HandleRequest(SOCKET clientSocket, const HttpRequest& httpRequest)
{
	const string codeLine = HttpServer::cleanAssemblyString(httpRequest.request.substr(5), false);
	const string pageHeader = R"(<!DOCTYPE html>
						<HTML prefix="og: http://ogp.me/ns#">
						<head>
						<title>Project - CS350 Assembler</title>
						<link rel = "icon" type = "image/png" href = "/favicon.png">
						<meta property="og:title" content="User has shared a line of code from VIM:"/>
						<meta property="og:image" content="http://www.vexal.us/vfbicon.png"/>
						<meta property="og:site_name" content="Vexal's Website"/>
						<meta property="og:url" content="http://www.vexal.us)" + httpRequest.request + R"("/>
						<meta property="og:type" content="website" />
						<meta property="og:description" content=")" + codeLine + R"("/>
    
						</head>
						<body>)";
	const string pageEnd = R"(</body>
						</html>)";
	const string fullPage = pageHeader + codeLine + pageEnd;

	const Page* const newPage = new const Page("asdf", "vim", fullPage);

	this->server->SendPage(newPage, clientSocket);
	delete newPage;
}