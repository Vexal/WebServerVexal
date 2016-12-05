#pragma once
#include <string>

class UserComment
{
public:
	const unsigned int id;
	const unsigned int threadId;
	const unsigned int userId;
	const std::string username;
	const std::string userIpAddress;
	const std::string timestamp;
	const std::string text;
};