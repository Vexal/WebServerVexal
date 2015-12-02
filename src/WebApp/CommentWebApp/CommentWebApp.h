#pragma once
#include "../WebApp.h"
#include "../../Logger/Logger.h"

class Folder;
class Page;
class UserDAO;
class CommentsDAO;

class CommentWebApp : public WebApp
{
private:
	const Folder* const rootDirectory;
	UserDAO* const userDAO;
	CommentsDAO* const commentsDAO;
	Logger log = Logger("CommentWebApp");

public:
	CommentWebApp(HttpServer* server, const Folder* const rootDirectory);
	virtual void HandleRequest(SOCKET clientSocket, const HttpRequest& httpRequest) override;
};