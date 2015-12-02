#pragma once
#include <string>

struct DbConfig
{
	const std::string dbAddress;
	const std::string dbUsername;
	const std::string dbPassword;
	const std::string dbName;

	static DbConfig Load(const std::string& file, const std::string& dbName);
};