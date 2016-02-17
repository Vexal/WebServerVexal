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
	dbConfig(DbConfig::Load("dbconfig.txt", "vexal"))
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

int DbUserDAO::GetUserId(const string& accountName, const string& password) const
{
	try
	{
		const MySqlConnection connection(this->driver, this->dbConfig);
		auto prep_stmt = connection.GetConnection()->prepareStatement("SELECT id FROM users WHERE username = ? AND password = ?");
		prep_stmt->setString(1, accountName);
		prep_stmt->setString(2, password);

		const auto result = prep_stmt->executeQuery();
		delete prep_stmt;

		if (!result->next())
		{
			delete result;
			throw InvalidCredentialsException();
		}

		const int id = result->getInt("id");
		delete result;
		return id;
	}
	catch (const sql::SQLException& e)
	{
		throw DataErrorException{ e.what() };
	}
}

User DbUserDAO::GetValidatedAccount(const string& accountName, const string& password, const bool createIfNotExist, bool& wasCreated)
{
	try
	{
		const MySqlConnection connection(this->driver, this->dbConfig);

		if (createIfNotExist)
		{
			//check if account exists
			auto prep_stmt = connection.GetConnection()->prepareStatement("SELECT id FROM users WHERE username = ?");
			prep_stmt->setString(1, accountName);

			sql::ResultSet* const result = prep_stmt->executeQuery();
			delete prep_stmt;

			const int resultCount = result->rowsCount();
			delete result;

			if (resultCount == 0)
			{
				//account does not exist.  attempt to create with provided credentials (someone could have gotten here first)
				auto const accountCreateStatement = connection.GetConnection()->prepareStatement("INSERT IGNORE INTO users (username, password) VALUES (?, ?)");
				accountCreateStatement->setString(1, accountName);
				accountCreateStatement->setString(2, password);

				const int updatecount = accountCreateStatement->executeUpdate();
				delete accountCreateStatement;
				wasCreated = updatecount == 1;
			}
			else
			{
				wasCreated = false;
			}
		}

		{
			//get user
			auto const prep_stmt = connection.GetConnection()->prepareStatement("SELECT id, username FROM users WHERE username = ? AND password = ?");
			prep_stmt->setString(1, accountName);
			prep_stmt->setString(2, password);

			const auto result = prep_stmt->executeQuery();
			delete prep_stmt;

			if (!result->next())
			{
				delete result;
				throw InvalidCredentialsException();
			}

			const int id = result->getInt("id");
			const string newUsername = result->getString("username");
			delete result;
			return{ id, newUsername };
		}
	}
	catch (const sql::SQLException& e)
	{
		throw DataErrorException{ e.what() };
	}
}

unordered_set<string> DbUserDAO::GetAccessTypes(const int userId) const
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

DbUserDAO::~DbUserDAO()
{

}

DbUserDAO* DbUserDAO::Create()
{
	static DbUserDAO singletonDAO;
	return &singletonDAO;
}
