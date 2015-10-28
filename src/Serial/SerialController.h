#pragma once
#ifdef _WIN32
#include <windows.h>
#endif
#include <mutex>

class SerialController
{
private:
#ifdef _WIN32
	//Serial comm handler
	HANDLE hSerial;
	//Get various information about the connection
	COMSTAT status;
	//Keep track of last error
	DWORD errors;
	LPCSTR portName = "COM3";
#endif
	bool connected;
	std::mutex serialMutex;
	static const int ARDUINO_WAIT_TIME = 2000;

private:
	SerialController();

public:
	bool WriteData(const char *buffer, unsigned int nbChar);
	int ReadData(char *buffer, unsigned int nbChar);
	bool InitializeSerialConnection();

	//returns singleton
	static SerialController* Create();
};