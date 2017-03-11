#include <iostream>
#include "../Serial/SerialController.h"
#include "HomeAutomationController.h"
#ifdef _WIN32
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <string>


#include <netdb.h>

#include <unistd.h>
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define SOCKET int
#endif
using namespace std;

HomeAutomationController::HomeAutomationController() :
	serialController(SerialController::Create())
{
	cout << "Initializing Serial Connection..." << endl;
	if (this->serialController->InitializeSerialConnection())
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
	if (this->serialController->WriteData(buf, 7))
	{
		return 0;
	}
	else
	{
		return -1;
	}
}

static const char* addr = "192.168.1.107";

#define DEFAULT_BUFLEN 512
int HomeAutomationController::ActivateGarageNetwork(string& resultStatus)
{
	//----------------------
	// Declare and initialize variables.
	char *sendbuf = "0 Garage";
	int iResult;
	SOCKET ConnectSocket = 0;
	struct sockaddr_in clientService;

	int recvbuflen = DEFAULT_BUFLEN;
	char recvbuf[DEFAULT_BUFLEN] = "";

	//----------------------
	// Create a SOCKET for connecting to server
	ConnectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

#ifdef _WIN32
	unsigned long timeout = 15000; //60 seconds
#else
	struct timeval timeout;
	timeout.tv_sec = 15;
	timeout.tv_usec = 0;
#endif
	setsockopt(ConnectSocket, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(timeout));
	if (ConnectSocket == INVALID_SOCKET) {
#ifdef _WIN32
		wprintf(L"socket failed with error: %ld\n", WSAGetLastError());
#endif
		return -1;
	}

	cout << "sending.." << endl;
	//----------------------
	// The sockaddr_in structure specifies the address family,
	// IP address, and port of the server to be connected to.
	clientService.sin_family = AF_INET;
	clientService.sin_addr.s_addr = inet_addr(addr);
	clientService.sin_port = htons(9191);

	//----------------------
	// Connect to server.
	iResult = connect(ConnectSocket, (sockaddr*)&clientService, sizeof(clientService));
	if (iResult == SOCKET_ERROR) {
#ifdef _WIN32
		cout << "connect failed with error:  " << endl << WSAGetLastError();
		closesocket(ConnectSocket);
#else
		close(ConnectSocket);
#endif
		return -1;
	}

	cout << "Connected..." << endl;
	//----------------------
	// Send an initial buffer
	iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
	if (iResult == SOCKET_ERROR) {
#ifdef _WIN32
		cout << "send failed with error: " << WSAGetLastError() << endl;
		closesocket(ConnectSocket);
#else
		close(ConnectSocket);
#endif
		return -1;
	}

	cout << "Bytes Sent: " << iResult << endl;

	// shutdown the connection since no more data will be sent
	iResult = shutdown(ConnectSocket, 1);
	if (iResult == SOCKET_ERROR) {
#ifdef _WIN32
		cout << WSAGetLastError();
		closesocket(ConnectSocket);
#else
		close(ConnectSocket);
#endif
			return -1;
	}

	// Receive until the peer closes the connection
	do {

		iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0)
			cout << "Bytes received: " << iResult;
		else if (iResult == 0)
			cout << "Connection closed" << endl;

	} while (iResult > 0);


	// close the socket
#ifdef _WIN32
	iResult = closesocket(ConnectSocket);
	if (iResult == SOCKET_ERROR) {
		cout << "close failed with error: " << WSAGetLastError();
		closesocket(ConnectSocket);
		return -1;
	}
#else
	close(ConnectSocket);
#endif
	return 0;
}

int HomeAutomationController::ActivateRoomAlarmNetwork(string& resultStatus)
{
	//----------------------
	// Declare and initialize variables.
	char *sendbuf = "s room alarm";
	int iResult;
	SOCKET ConnectSocket = 0;
	struct sockaddr_in clientService;

	int recvbuflen = DEFAULT_BUFLEN;
	char recvbuf[DEFAULT_BUFLEN] = "";

	//----------------------
	// Create a SOCKET for connecting to server
	ConnectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

#ifdef _WIN32
	unsigned long timeout = 15000; //60 seconds
#else
	struct timeval timeout;
	timeout.tv_sec = 15;
	timeout.tv_usec = 0;
#endif
	setsockopt(ConnectSocket, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(timeout));
	if (ConnectSocket == INVALID_SOCKET) {
#ifdef _WIN32
		wprintf(L"socket failed with error: %ld\n", WSAGetLastError());
#endif
		return -1;
	}

	cout << "sending.." << endl;
	//----------------------
	// The sockaddr_in structure specifies the address family,
	// IP address, and port of the server to be connected to.
	clientService.sin_family = AF_INET;
	clientService.sin_addr.s_addr = inet_addr(addr);
	clientService.sin_port = htons(9191);

	//----------------------
	// Connect to server.
	iResult = connect(ConnectSocket, (sockaddr*)&clientService, sizeof(clientService));
	if (iResult == SOCKET_ERROR) {
#ifdef _WIN32
		cout << "connect failed with error:  " << endl << WSAGetLastError();
		closesocket(ConnectSocket);
#else
		cout << "connect failed" << endl;
		close(ConnectSocket);
#endif
		return -1;
	}

	cout << "Connected..." << endl;
	//----------------------
	// Send an initial buffer
	iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
	if (iResult == SOCKET_ERROR) {
#ifdef _WIN32
		cout << "send failed with error: " << WSAGetLastError() << endl;
		closesocket(ConnectSocket);
#else
		close(ConnectSocket);
#endif
		return -1;
	}

	cout << "Bytes Sent: " << iResult << endl;

	// shutdown the connection since no more data will be sent
	iResult = shutdown(ConnectSocket, 1);
	if (iResult == SOCKET_ERROR) {
#ifdef _WIN32
		cout << WSAGetLastError();
		closesocket(ConnectSocket);
#else
		close(ConnectSocket);
#endif
			return -1;
	}

	// Receive until the peer closes the connection
	do {

		iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0)
			cout << "Bytes received: " << iResult;
		else if (iResult == 0)
			cout << "Connection closed" << endl;

	} while (iResult > 0);


	// close the socket
#ifdef _WIN32
	iResult = closesocket(ConnectSocket);
	if (iResult == SOCKET_ERROR) {
		cout << "close failed with error: " << WSAGetLastError();
		closesocket(ConnectSocket);
		return -1;
	}
#else
	close(ConnectSocket);
#endif
	return 0;
}

HomeAutomationController* HomeAutomationController::Create()
{
	static HomeAutomationController singletonController;
	return &singletonController;
}
