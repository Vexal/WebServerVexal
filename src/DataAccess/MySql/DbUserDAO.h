#pragma once
#include "../UserDAO.h"
#include "DbConfig.h"
#include "MySqlDAO.h"

namespace sql
{
	class Driver;
	class Connection;
}

class DbUserDAO : public UserDAO, public MySqlDAO
{
private:
	DbUserDAO();

public:
	virtual int CreateAccount(const std::string& accountName, const std::string& password) override;
	virtual int GetUserId(const std::string& accountName, const std::string& password) const override;
	virtual User GetValidatedAccount(const std::string& accountName, const std::string& password, const bool createIfNotExist, bool& wasCreated) override;
	virtual std::unordered_set<std::string> GetAccessTypes(const int userId) const override;
	virtual ~DbUserDAO();

	static DbUserDAO* Create();
};