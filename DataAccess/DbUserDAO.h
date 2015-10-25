#pragma once
#include "UserDAO.h"
#include "MySql/DbConfig.h"

namespace sql
{
	class Driver;
	class Connection;
}

class DbUserDAO : public UserDAO
{
private:
	const DbConfig dbConfig;
	sql::Driver* driver = nullptr;

protected:
	DbUserDAO();

public:
	virtual int CreateAccount(const std::string& accountName, const std::string& password) override;
	virtual int GetUserId(const std::string& accountName, const std::string& password) override;
	virtual std::unordered_set<std::string> GetAccessTypes(const int userId) override;
	virtual ~DbUserDAO();

	static DbUserDAO* Create();

private:
	static DbConfig loadConfig();
};