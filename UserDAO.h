#pragma once
#include <string>

class UserDAO
{
protected:
	bool isValid = false;

public:
	//return new account id, or -1 if fail
	virtual int CreateAccount(const std::string& accountName, const std::string& password) = 0;

	bool IsValid() const { return this->isValid; }
	virtual ~UserDAO() {}
};

struct DuplicateAccountException {};
struct DataErrorException { const std::string error; };
