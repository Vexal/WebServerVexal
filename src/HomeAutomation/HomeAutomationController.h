#pragma once
#include <mutex>
#include <string>

class HomeAutomationController
{
protected:
	std::mutex serialMutex;

	HomeAutomationController();

public:
	//return 0 if successful.
	int ActivateGarage(std::string& resultStatus);

public:
	//returns singleton
	static HomeAutomationController* Create();
};