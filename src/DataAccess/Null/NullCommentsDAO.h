#pragma once
#include "../CommentsDAO.h"

class NullCommentsDAO : public CommentsDAO
{
private:
	NullCommentsDAO() {};

public:
	virtual UserCommentsThread GetThread(const unsigned int threadId) const override;
	virtual UserCommentsThread GetThread(const std::string& threadKey) const override;
	virtual void PostComment(const std::string& threadKey, const User& user, const std::string& userIpAddress, const std::string& comment) const override {};
	static NullCommentsDAO* Create();
};