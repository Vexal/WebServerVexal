#include <cppconn/driver.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/exception.h>

#include "../../CommentsSystem/CommentsException.h"
#include "../DataErrorException.h"
#include "DbCommentsDAO.h"
#include "MySqlConnection.h"

using namespace std;

DbCommentsDAO::DbCommentsDAO() :
	dbConfig(DbConfig::Load("dbconfig.txt", "vexal"))
{
	this->driver = get_driver_instance(); //TODO learn if I'm supposed to do this for every DAO
}

UserCommentsThread DbCommentsDAO::GetThread(const unsigned int threadId) const
{
	return{0};
}

UserCommentsThread DbCommentsDAO::GetThread(const string& threadKey) const
{
	const char* const getThreadsSql = "SELECT id "
									  "FROM tblthreads "
									  "WHERE thread_key = ?";

	const char* const getCommentsForThreadSql = "SELECT tblcomments.id as id, tblcomments.thread_id as thread_id, tblcomments.comment as comment, tblcomments.user_id as user_id, tblcomments.user_ip as user_ip, tblcomments.date as date, users.username as username "
												"FROM tblcomments "
												"JOIN users ON tblcomments.user_id = users.id "
												"WHERE thread_id = ?";
	try
	{
		const MySqlConnection connection(this->driver, this->dbConfig);

		sql::PreparedStatement* const getThreadsStatement = connection.GetConnection()->prepareStatement(getThreadsSql);
		getThreadsStatement->setString(1, threadKey);

		sql::ResultSet* const result = getThreadsStatement->executeQuery();
		delete getThreadsStatement;

		if (result->rowsCount() != 1)
		{
			throw CommentsException{ "Error:  Invalid thread key." };
		}

		result->next();
		const unsigned int threadId = result->getUInt("id");

		delete result;

		sql::PreparedStatement* const getCommentsStatement = connection.GetConnection()->prepareStatement(getCommentsForThreadSql);
		getCommentsStatement->setInt(1, threadId);

		sql::ResultSet* const commentsResultSet = getCommentsStatement->executeQuery();
		delete getCommentsStatement;

		vector<UserComment> comments;
		while (commentsResultSet->next())
		{
			comments.push_back({
				commentsResultSet->getUInt("id"),
				commentsResultSet->getUInt("thread_id"),
				commentsResultSet->getUInt("user_id"),
				commentsResultSet->getString("username"),
				commentsResultSet->getString("user_ip"),
				commentsResultSet->getString("date"),
				commentsResultSet->getString("comment"),
			});
		}

		delete commentsResultSet;

		return{ threadId, comments };
	}
	catch (const sql::SQLException& e)
	{
		throw DataErrorException{ e.what() };
	}

	return{0};
}

void DbCommentsDAO::PostComment(const string& threadKey, const User& user, const string& userIpAddress, const string& comment) const
{
	const char* const getThreadsSql = "SELECT id "
		"FROM tblthreads "
		"WHERE thread_key = ?";

	const char* const insertCommentSql = "INSERT INTO tblcomments(thread_id,comment,user_id,user_ip) "
										 "VALUES(?, ?, ?, ?)";
	try
	{
		const MySqlConnection connection(this->driver, this->dbConfig);

		sql::PreparedStatement* const getThreadsStatement = connection.GetConnection()->prepareStatement(getThreadsSql);
		getThreadsStatement->setString(1, threadKey);

		sql::ResultSet* const result = getThreadsStatement->executeQuery();
		delete getThreadsStatement;

		if (result->rowsCount() != 1)
		{
			throw CommentsException{ "Error:  Invalid thread key." };
		}

		result->next();
		const unsigned int threadId = result->getUInt("id");

		delete result;

		sql::PreparedStatement* const postCommentStatement = connection.GetConnection()->prepareStatement(insertCommentSql);
		postCommentStatement->setInt(1, threadId);
		postCommentStatement->setString(2, comment);
		postCommentStatement->setInt(3, user.id);
		postCommentStatement->setString(4, userIpAddress);

		const int updateCount = postCommentStatement->executeUpdate();
		delete postCommentStatement;
	}
	catch (const sql::SQLException& e)
	{
		throw DataErrorException{ e.what() };
	}
}

DbCommentsDAO* DbCommentsDAO::Create()
{
	static DbCommentsDAO singletonDAO;
	return &singletonDAO;
}
