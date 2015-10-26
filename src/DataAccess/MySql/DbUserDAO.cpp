#include <iostream>
#include <fstream>
#include <cppconn/driver.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/exception.h>

#include "DbUserDAO.h"
#include "MySqlConnection.h"
#include "../DataErrorException.h"
#include "../UserDAO.h"

using namespace std;

DbUserDAO::DbUserDAO() :
	dbConfig(DbUserDAO::loadConfig())
{
	this->driver = get_driver_instance();
}

int DbUserDAO::CreateAccount(const string& accountName, const string& password)
{
	try
	{
		const MySqlConnection connection(this->driver, this->dbConfig);
		auto prep_stmt = connection.GetConnection()->prepareStatement("INSERT IGNORE INTO users (username, password) VALUES (?, ?)");
		prep_stmt->setString(1, accountName);
		prep_stmt->setString(2, password);

		const auto updatecount = prep_stmt->executeUpdate();
		delete prep_stmt;

		if (updatecount == 0)
		{
			throw DuplicateAccountException();
		}
	}
	catch (const sql::SQLException& e)
	{
		const DataErrorException ex = { e.what() };
		throw ex;
	}

	return 0;
}

int DbUserDAO::GetUserId(const string& accountName, const string& password)
{
	try
	{
		const MySqlConnection connection(this->driver, this->dbConfig);
		auto prep_stmt = connection.GetConnection()->prepareStatement("SELECT id FROM users WHERE username = ? AND password = ?");
		prep_stmt->setString(1, accountName);
		prep_stmt->setString(2, password);

		const auto result = prep_stmt->executeQuery();
		delete prep_stmt;

		if (result->rowsCount() != 1)
		{
			delete result;
			throw InvalidCredentialsException();
		}
		result->next();
		const int id = result->getInt("id");
		delete result;
		return id;
	}
	catch (const sql::SQLException& e)
	{
		throw DataErrorException{ e.what() };
	}
}

unordered_set<string> DbUserDAO::GetAccessTypes(const int userId)
{
	try
	{
		const MySqlConnection connection(this->driver, this->dbConfig);
		auto prep_stmt = connection.GetConnection()->prepareStatement("SELECT access FROM tbluseraccess WHERE user_id = ?");
		prep_stmt->setInt(1, userId);

		const auto result = prep_stmt->executeQuery();
		delete prep_stmt;

		unordered_set<string> accessTypes;

		while (result->next())
		{
			const string access = result->getString("access");
			accessTypes.insert(access);
		}

		delete result;
		return accessTypes;
	}
	catch (const sql::SQLException& e)
	{
		throw DataErrorException{ e.what() };
	}
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

		return { dbAddress, dbUsername, dbPassword, "vexal" };
	}

	return{ "error" };
}

DbUserDAO::~DbUserDAO()
{

}

DbUserDAO* DbUserDAO::Create()
{
	static DbUserDAO* const singletonDAO = new DbUserDAO();
	return singletonDAO;
}
