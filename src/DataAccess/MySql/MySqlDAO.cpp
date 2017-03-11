#ifndef _DEBUG
#include <cppconn/driver.h>
#endif
#include "MySqlDAO.h"
#include "DbConfig.h"

#ifndef _DEBUG
MySqlDAO::MySqlDAO() :
	dbConfig(DbConfig::Load("dbconfig.txt", "vexal"))
{
	this->driver = get_driver_instance(); //TODO learn if I'm supposed to do this for every DAO
}

MySqlConnection MySqlDAO::getConnection() const
{
	return MySqlConnection(this->driver, this->dbConfig);
}
#else
MySqlDAO::MySqlDAO() {}
#endif
