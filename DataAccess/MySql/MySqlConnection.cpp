#include <iostream>
#include <cppconn/driver.h>
#include <cppconn/statement.h>
#include "MySqlConnection.h"
#include "../DataErrorException.h"

using namespace std;

MySqlConnection::MySqlConnection(sql::Driver* const driver, const DbConfig& dbConfig, const bool threaded) :
	driver(driver),
	dbConfig(dbConfig),
	threaded(threaded)
{
	if (this->threaded)
	{
		this->driver->threadInit();
	}

	sql::Statement* statement = nullptr;
	bool successful = false;
	try
	{
		if (this->dbConfig.dbAddress == "error")
		{
			throw "Unable to connect.";
		}

		cout << "Initializing database connection '" << this->dbConfig.dbAddress << "'..." << endl;
		this->connection = this->driver->connect(this->dbConfig.dbAddress, this->dbConfig.dbUsername, this->dbConfig.dbPassword);

		statement = this->connection->createStatement();
		statement->execute("USE " + this->dbConfig.dbName);
		cout << "Database connection established." << endl;
		successful = true;
	}
	catch (...)
	{
		cout << "ERROR: Unable to establish database connection." << endl;
	}

	if (statement != nullptr)
	{
		delete statement;
	}

	if (!successful)
	{
		this->cleanUp();
		throw DataErrorException({ "Unable to connect to database." });
	}
}

MySqlConnection::~MySqlConnection()
{
	this->cleanUp();
}

void MySqlConnection::cleanUp()
{
	if (this->threaded)
	{
		this->driver->threadEnd();
	}

	if (this->connection != nullptr)
	{
		if (this->connection->isValid())
		{
			this->connection->close();
		}

		delete this->connection;
	}
}
