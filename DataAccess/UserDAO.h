#pragma once
#include <string>
#include <unordered_set>

class UserDAO
{
protected:
	bool isValid = false;

public:
	//return new account id, or -1 if fail
	virtual int CreateAccount(const std::string& accountName, const std::string& password) = 0;
	//returns user id if valid, else -1
	virtual int GetUserId(const std::string& accountName, const std::string& password) = 0;
	virtual std::unordered_set<std::string> GetAccessTypes(const int userId) = 0;
	bool IsValid() const { return this->isValid; }
	virtual ~UserDAO() {}
};

struct DuplicateAccountException {};
struct InvalidCredentialsException {};
