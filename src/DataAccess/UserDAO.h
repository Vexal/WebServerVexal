#pragma once
#include <string>
#include <unordered_set>
#include "../User/User.h"

class UserDAO
{
protected:
	bool isValid = false;

public:
	//return new account id, or -1 if fail
	virtual int CreateAccount(const std::string& accountName, const std::string& password) = 0;
	//returns user id if valid, else -1
	virtual int GetUserId(const std::string& accountName, const std::string& password) const = 0;
	//returns user object if valid or created, else nullptr
	virtual User GetValidatedAccount(const std::string& accountName, const std::string& password, const bool createIfNotExist, bool& wasCreated) = 0;
	virtual std::unordered_set<std::string> GetAccessTypes(const int userId) const = 0;
	bool IsValid() const { return this->isValid; }
	virtual ~UserDAO() {}
};

struct DuplicateAccountException {};
struct InvalidCredentialsException {};
