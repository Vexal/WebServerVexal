#include <thread>
#include <fstream>
#include <iostream>
#ifdef _WIN32
#include <Windows.h>
#endif
#include "../Util.h"
#include "Logger.h"

using namespace std;

mutex Logger::logQueueMutex;
queue<Logger::LogTask> Logger::logTaskQueue; //queue log entries to a separate thread
condition_variable Logger::logConditionVariable;

Logger::Logger(const string& name) :
	name(name)
{
	static const bool initialized = [&]() { //this is cool.
		thread logWriteThread(&Logger::logThreadFunction);
		logWriteThread.detach();
		return true;
	}();
}

void Logger::info(const string& logEntry)
{
	Logger::logQueueMutex.lock();
	Logger::logTaskQueue.push({ LOG_INFO, "[" + this->name + "] " + logEntry });
	Logger::logQueueMutex.unlock();
	Logger::logConditionVariable.notify_all();
}

void Logger::error(const string& logEntry)
{
	Logger::logQueueMutex.lock();
	Logger::logTaskQueue.push({ LOG_ERROR, "[" + this->name + "] " + logEntry });
	Logger::logQueueMutex.unlock();
	Logger::logConditionVariable.notify_all();
}

void Logger::append(const string& file, const string& logEntry)
{
	Logger::logQueueMutex.lock();
	Logger::logTaskQueue.push({ LOG_APPEND, logEntry, file });
	Logger::logQueueMutex.unlock();
	Logger::logConditionVariable.notify_all();
}

void Logger::logThreadFunction()
{
	//TODO: learn if it's okay to leave files open or if they should be closed after every write
	static ofstream infoLog("Application Logs/application.log", ios_base::app);
	static ofstream errorLog("Application Logs/application-error.log", ios_base::app);

	if (!infoLog.is_open() || !errorLog.is_open())
	{
		cout << "Error initializing log files." << endl;
		return;
	}

	while (true)
	{
		unique_lock<mutex> lk(logQueueMutex);
		//bug in g++; won't compile without capturing;
		Logger::logConditionVariable.wait(lk, [&]() {return !Logger::logTaskQueue.empty();}); 

		const LogTask logTask = Logger::logTaskQueue.front();
		Logger::logTaskQueue.pop();
		lk.unlock();
		switch (logTask.logType)
		{
		case LOG_INFO:
			infoLog << Util::CurrentDateTime() << " INFO: " << logTask.logEntry << endl;
			break;
		case LOG_ERROR:
			errorLog << Util::CurrentDateTime() << " ERROR: " << logTask.logEntry << endl;
			break;
		case LOG_APPEND:
		{
			ofstream logFile;
			logFile.open(logTask.fileName, ios_base::app);
			logFile << logTask.logEntry << endl;
			logFile.close();
		}
			break;
		}
	}
}
