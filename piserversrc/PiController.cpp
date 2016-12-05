#include <iostream>
#include "PiController.h"
#ifndef _WIN32
#include <stdlb.h>
#include <wiringPi.h>
#endif

using namespace std;

PiController::PiController()
{
#ifndef _WIN32
	static const bool isInitialized = []() {
		wiringPiSetup();
		pinMode(0, OUTPUT);
		return true;
	}();
#endif
}
void PiController::doEverything(const std::string& thingsToDo)
{
	int pin = -1;
	bool playSound = false;
	if (thingsToDo[0] == '0')
		pin = 0;
	else if (thingsToDo[0] == '1')
		pin = 1;
	else if (thingsToDo[0] == 's')
		playSound = true;
	if (pin >= 0)
	{
#ifndef _WIN32
		digitalWrite(pin, HIGH);
		delay(1000);
		digitalWrite(pin, LOW);
#endif
	}
	else if (playSound)
	{
#ifndef _WIN32
		system("omxplayer ~/dev/WebServerVexal/alarm.wav");
#endif
	}
	cout << "Did everything with Pi Controller: " << thingsToDo << endl;
}
