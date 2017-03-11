#pragma once
#include "../CommentsDAO.h"
#include "MySqlDAO.h"

#ifndef _DEBUG
namespace sql
{
	class Driver;
	class Connection;
}
#endif

class DbCommentsDAO : public CommentsDAO, public MySqlDAO
{
private:
	DbCommentsDAO();

public:
	virtual UserCommentsThread GetThread(const unsigned int threadId) const override;
	virtual UserCommentsThread GetThread(const std::string& threadKey) const override;
	virtual void PostComment(const std::string& threadKey, const User& user, const std::string& userIpAddress, const std::string& comment) const override;
	static DbCommentsDAO* Create();
};