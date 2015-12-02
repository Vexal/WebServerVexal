#pragma once
#include <string>
#include <unordered_set>
#include "../CommentsSystem/UserCommentsThread.h"
#include "../User/User.h"

class CommentsDAO
{
public:
	//throws DataErrorException, CommentsException
	virtual UserCommentsThread GetThread(const unsigned int threadId) const = 0;
	virtual UserCommentsThread GetThread(const std::string& threadKey) const = 0;
	virtual void PostComment(const std::string& threadKey, const User& user, const std::string& userIpAddress, const std::string& comment) const = 0;
	virtual ~CommentsDAO() {}

	//optional
	static std::string GetThreadKey(const std::string& input);
};
