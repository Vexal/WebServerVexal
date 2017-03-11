#pragma once
#include "MySqlConnection.h"

class MySqlDAO
{
private:
	const DbConfig dbConfig;
#ifndef _DEBUG
	sql::Driver* driver = nullptr;
#endif

protected:
	MySqlDAO();
#ifndef _DEBUG
	MySqlConnection getConnection() const;
#endif
};