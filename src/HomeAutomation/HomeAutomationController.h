#pragma once
#include <mutex>
#include <string>

class SerialController;

class HomeAutomationController
{
protected:
	SerialController* const serialController;
	std::mutex serialMutex;

	HomeAutomationController();

public:
	//return 0 if successful.
	int ActivateGarage(std::string& resultStatus);
	int ActivateGarageNetwork(std::string& resultStatus);
	int ActivateRoomAlarmNetwork(std::string& resultStatus);

public:
	//returns singleton
	static HomeAutomationController* Create();
};