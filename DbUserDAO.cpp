#include <iostream>
#include <cppconn/driver.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/exception.h>
#include <fstream>

#include "DbUserDAO.h"
using namespace std;

DbUserDAO::DbUserDAO() :
	dbConfig(DbUserDAO::loadConfig())
{
	try
	{
		if (this->dbConfig.dbAddress == "error")
		{
			throw "Unable to connect.";
		}

		cout << "Initializing database connection '" << this->dbConfig.dbAddress << "'..." << endl;
		this->driver = get_driver_instance();
		this->con = this->driver->connect(this->dbConfig.dbAddress, this->dbConfig.dbUsername, this->dbConfig.dbPassword);
		this->isValid = true;
		cout << "Database connection established." << endl;
	}
	catch (...)
	{
		cout << "ERROR: Unable to establish database connection." << endl;
	}

	if (this->isValid)
	{
		auto stmt = this->con->createStatement();
		stmt->execute("USE vexal");
		auto res = stmt->executeQuery("SELECT * from users");
		while (res->next()) {
			try {
				auto id = res->getInt("id");
				sql::SQLString username = res->getString("username");
				cout << "id: " << id << " username: " << username << endl;
			}
			catch (...) {

			}
		}

		delete res;
		delete stmt;
	}
}

int DbUserDAO::CreateAccount(const string& accountName, const string& password)
{
	try
	{
		auto prep_stmt = con->prepareStatement("INSERT IGNORE INTO users (username, password) VALUES (?, ?)");
		prep_stmt->setString(1, accountName);
		prep_stmt->setString(2, password);

		auto updatecount = prep_stmt->executeUpdate();
		if (updatecount == 0)
		{
			throw DuplicateAccountException();
		}

		delete prep_stmt;
	}
	catch (sql::SQLException e)
	{
		const DataErrorException ex = { e.what() };
		throw ex;
	}

	return 0;
}

DbConfig DbUserDAO::loadConfig()
{
	ifstream configFile("dbconfig.txt");
	if (configFile.is_open())
	{
		string dbAddress, dbUsername, dbPassword;
		configFile >> dbAddress;
		configFile >> dbUsername;
		configFile >> dbPassword;

		configFile.close();

		return { dbAddress, dbUsername, dbPassword };
	}

	return{ "error" };
}

DbUserDAO::~DbUserDAO()
{
	this->con->close();
	delete this->con;
}
