#pragma once
#include "../CommentsDAO.h"
#include "DbConfig.h"

namespace sql
{
	class Driver;
	class Connection;
}

class DbCommentsDAO : public CommentsDAO
{
private:
	const DbConfig dbConfig;
	sql::Driver* driver = nullptr;

private:
	DbCommentsDAO();

public:
	virtual UserCommentsThread GetThread(const unsigned int threadId) const override;
	virtual UserCommentsThread GetThread(const std::string& threadKey) const override;
	virtual void PostComment(const std::string& threadKey, const User& user, const std::string& userIpAddress, const std::string& comment) const override;
	static DbCommentsDAO* Create();
};