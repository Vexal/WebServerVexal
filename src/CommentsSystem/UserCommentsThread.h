#pragma once
#include <vector>
#include "UserComment.h"

class UserCommentsThread
{
public:
	const unsigned int threadId;
	const std::vector<UserComment> threadComments;

	std::string GenerateFullHtml();
	static std::string GenerateCommentHtml(const UserComment& comment);
};