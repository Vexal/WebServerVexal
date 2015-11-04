#ifdef _WIN32
#else
#include <string.h>     // string function definitions
#include <unistd.h>     // UNIX standard function definitions
#include <fcntl.h>      // File control definitions
#include <errno.h>      // Error number definitions
#include <termios.h>    // POSIX terminal control definitions
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>

#include "SerialController.h"

using namespace std;

SerialController::SerialController()
{

}

SerialController* SerialController::Create()
{
	static SerialController singletonController;
	return &singletonController;
}

bool SerialController::InitializeSerialConnection()
{
	this->serialMutex.lock();
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
#else //linux
	USB = open("/dev/ttyACM0", O_RDWR | O_NOCTTY | O_NDELAY);
	struct termios tty;
	struct termios tty_old;
	memset(&tty, 0, sizeof tty);

	/* Error Handling */
	if (tcgetattr(USB, &tty) != 0) {
		std::cout << "Error " << errno << " from tcgetattr: " << strerror(errno) << std::endl;
		return false;
	}

	usleep(3500000);

	cout << "successful" << endl;

	/* Save old tty parameters */
	tty_old = tty;

	/* Set Baud Rate */
	cfsetospeed(&tty, (speed_t)B9600);
	cfsetispeed(&tty, (speed_t)B9600);

	/* Setting other Port Stuff */
	tty.c_cflag &= ~PARENB;            // Make 8n1
	tty.c_cflag &= ~CSTOPB;
	tty.c_cflag &= ~CSIZE;
	tty.c_cflag |= CS8;

	tty.c_cflag &= ~CRTSCTS;           // no flow control
	tty.c_cc[VMIN] = 1;                  // read doesn't block
	tty.c_cc[VTIME] = 5;                  // 0.5 seconds read timeout
	tty.c_cflag |= CREAD | CLOCAL;     // turn on READ & ignore ctrl lines

									   /* Make raw */
	cfmakeraw(&tty);

	/* Flush Port, then applies attributes */
	tcflush(USB, TCIFLUSH);
	if (tcsetattr(USB, TCSANOW, &tty) != 0) {
		std::cout << "Error " << errno << " from tcsetattr" << std::endl;
		return false;
	}

#endif
	this->serialMutex.unlock();
	return connected;
}

bool SerialController::WriteData(const char *buffer, unsigned int nbChar)
{
	this->serialMutex.lock();
	if (!connected)
	{
		return false;
	}

#ifdef _WIN32
	DWORD bytesSend;

	//Try to write the buffer on the Serial port
	if (!WriteFile(hSerial, (void *)buffer, nbChar, &bytesSend, 0))
	{
		//In case it don't work get comm error and return false
		ClearCommError(hSerial, &errors, &status);

		return false;
	}
	else
		return true;
#else
	int written = 1;	
	int ind = 0;

	while (written > 0 && written - 1 < nbChar)
	{
		written = write(USB, &buffer[ind], 1);
		ind += written;
	}

#endif
	this->serialMutex.unlock();
}

int SerialController::ReadData(char *buffer, unsigned int nbChar)
{
	this->serialMutex.lock();
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
	if (status.cbInQue > 0)
	{
		//If there is we check if there is enough data to read the required number
		//of characters, if not we'll read only the available characters to prevent
		//locking of the application.
		if (status.cbInQue > nbChar)
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
	this->serialMutex.unlock();
}