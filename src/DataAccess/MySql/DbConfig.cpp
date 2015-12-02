#include <fstream>
#include "DbConfig.h"

using namespace std;

DbConfig DbConfig::Load(const std::string & file, const std::string & dbName)
{
	ifstream configFile(file);
	if (configFile.is_open())
	{
		string dbAddress, dbUsername, dbPassword;
		configFile >> dbAddress;
		configFile >> dbUsername;
		configFile >> dbPassword;

		configFile.close();

		return{ dbAddress, dbUsername, dbPassword, dbName };
	}

	return{ "error" };
}
