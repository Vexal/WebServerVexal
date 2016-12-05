#pragma once
#include "MySqlConnection.h"

class MySqlDAO
{
private:
	const DbConfig dbConfig;
	sql::Driver* driver = nullptr;

protected:
	MySqlDAO();
	MySqlConnection getConnection() const;
};