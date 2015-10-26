#include <iostream>
#include "..\Serial\SerialHandler.h"
#include "HomeAutomationController.h"

using namespace std;

HomeAutomationController::HomeAutomationController()
{
	cout << "Initializing Serial Connection..." << endl;
	if (Serial::InitializeSerialConnection())
	{
		cout << "Serial connection initialization successful." << endl;
	}
	else
	{
		cout << "ERROR: Serial initialization failed." << endl;
	}
}

int HomeAutomationController::ActivateGarage(string& resultStatus)
{
	cout << "Sending TOGGLE command to serial...";
	char buf[7] = "toggle";
	buf[6] = '\n';
	if (Serial::WriteData(buf, 7))
	{
		return 0;
	}
	else
	{
		return -1;
	}
}

HomeAutomationController* HomeAutomationController::Create()
{
	static HomeAutomationController* const singletonController = new HomeAutomationController();
	return singletonController;
}
