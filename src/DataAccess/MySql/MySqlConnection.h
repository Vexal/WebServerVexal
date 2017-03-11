#pragma once
#include "DbConfig.h"

#ifndef _DEBUG
namespace sql
{
	class Driver;
	class Connection;
}
#endif

//connection is initialized immediately upon construction and closed immediately upon destruction
//TODO: learn if this is the correct way to do things. Also learn about correctly using driver->threadInit()
class MySqlConnection
{
#ifndef _DEBUG
private:
	sql::Connection* connection = nullptr;
	sql::Driver* const driver;
	const DbConfig dbConfig;
	const bool threaded;

public:
	//Throws DataErrorException if initialization of connection unsuccessful.
	MySqlConnection(sql::Driver* const driver, const DbConfig& dbConfig, const bool threaded = true);
	~MySqlConnection();

	sql::Connection* GetConnection() const { return this->connection; }

private:
	void cleanUp();
#endif
};