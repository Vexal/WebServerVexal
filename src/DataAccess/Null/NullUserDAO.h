#pragma once
#include "../UserDAO.h"

class NullUserDAO : public UserDAO
{
public:
	virtual int CreateAccount(const std::string& accountName, const std::string& password) override;
};