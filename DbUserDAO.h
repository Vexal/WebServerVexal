#pragma once
#include "UserDAO.h"

namespace sql
{
	class Driver;
	class Connection;
}

struct DbConfig
{
	const std::string dbAddress;
	const std::string dbUsername;
	const std::string dbPassword;
};

class DbUserDAO : public UserDAO
{
private:
	const DbConfig dbConfig;
	sql::Driver *driver = nullptr;
	sql::Connection *con = nullptr;

public:
	DbUserDAO();
	int CreateAccount(const std::string& accountName, const std::string& password) override;
	virtual ~DbUserDAO();

private:
	static DbConfig loadConfig();
};