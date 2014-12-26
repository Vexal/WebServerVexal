#include "SerialHandler.h"
#ifdef _WIN32
#include <windows.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#define ARDUINO_WAIT_TIME 2000

//Connection status
bool connected;

#ifdef _WIN32
//Serial comm handler
HANDLE hSerial;
//Get various information about the connection
COMSTAT status;
//Keep track of last error
DWORD errors;
LPCSTR portName = "COM4";
#endif

using namespace std;

bool InitializeSerialConnection()
{
	if (connected)
	{
		return true;
	}

#ifdef _WIN32
	hSerial = CreateFileA(portName,
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (hSerial == INVALID_HANDLE_VALUE)
	{
		//If not success full display an Error
		if (GetLastError() == ERROR_FILE_NOT_FOUND)
		{

			//Print Error if neccessary
			printf("ERROR: Handle was not attached. Reason: %s not available.\n", portName);

		}
		else
		{
			printf("ERROR!!!");
		}
	}
	else
	{
		//If connected we try to set the comm parameters
		DCB dcbSerialParams = { 0 };

		//Try to get the current
		if (!GetCommState(hSerial, &dcbSerialParams))
		{
			//If impossible, show an error
			printf("failed to get current serial parameters!");
		}
		else
		{
			//Define serial connection parameters for the arduino board
			dcbSerialParams.BaudRate = CBR_9600;
			dcbSerialParams.ByteSize = 8;
			dcbSerialParams.StopBits = ONESTOPBIT;
			dcbSerialParams.Parity = NOPARITY;

			//Set the parameters and check for their proper application
			if (!SetCommState(hSerial, &dcbSerialParams))
			{
				printf("ALERT: Could not set Serial Port parameters");
			}
			else
			{
				//If everything went fine we're connected
				connected = true;
				//We wait 2s as the arduino board will be reseting
				Sleep(ARDUINO_WAIT_TIME);
			}
		}
	}

	/*while (true)
	{
		string s;
		cin >> s;
		char buf[100];// = s.c_str();
		for (int a = 0; a < s.size(); ++a)
		{
			buf[a] = s[a];
		}
		buf[s.size()] = '\n';
		WriteData(buf, s.size() + 1);

	}
	*/
	return connected;
#endif
}

bool WriteData(const char *buffer, unsigned int nbChar)
{
	if (!connected)
	{
		return false;
	}

#ifdef _WIN32
	DWORD bytesSend;

	//Try to write the buffer on the Serial port
	if (!WriteFile(hSerial, (void *) buffer, nbChar, &bytesSend, 0))
	{
		//In case it don't work get comm error and return false
		ClearCommError(hSerial, &errors, &status);

		return false;
	}
	else
		return true;
#endif
}

int ReadData(char *buffer, unsigned int nbChar)
{
	if (!connected)
	{
		return false;
	}

#ifdef _WIN32
	//Number of bytes we'll have read
	DWORD bytesRead;
	//Number of bytes we'll really ask to read
	unsigned int toRead;

	//Use the ClearCommError function to get status info on the Serial port
	ClearCommError(hSerial, &errors, &status);

	//Check if there is something to read
	if (status.cbInQue>0)
	{
		//If there is we check if there is enough data to read the required number
		//of characters, if not we'll read only the available characters to prevent
		//locking of the application.
		if (status.cbInQue>nbChar)
		{
			toRead = nbChar;
		}
		else
		{
			toRead = status.cbInQue;
		}

		//Try to read the require number of chars, and return the number of read bytes on success
		if (ReadFile(hSerial, buffer, toRead, &bytesRead, NULL) && bytesRead != 0)
		{
			return bytesRead;
		}

	}

	//If nothing has been read, or that an error was detected return -1
	return -1;
#endif

}