#pragma once
#include <mutex>
#include <queue>
#include <string>
#include <condition_variable>

class Logger
{
private:
	enum LogType
	{
		LOG_ERROR,
		LOG_INFO,
		LOG_APPEND //append requires file name
	};

	struct LogTask
	{
		const LogType logType;
		const std::string logEntry;
		const std::string fileName;
	};

	static std::mutex logQueueMutex;
	static std::queue<LogTask> logTaskQueue; //queue log entries to a separate thread
	static std::condition_variable logConditionVariable;
	const std::string name; //name associated with output in logs
	
public:
	Logger(const std::string& name);
	
	void info(const std::string& logEntry);
	void error(const std::string& logEntry);
	void append(const std::string& file, const std::string& logEntry);

private:
	static void logThreadFunction();
};