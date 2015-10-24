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
	int CreateAccount(const std::string& accountName, const std::string& password) override;
	virtual ~DbUserDAO();

	static DbUserDAO* Create();

private:
	static DbConfig loadConfig();
};