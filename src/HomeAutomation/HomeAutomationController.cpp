#include <iostream>
#include "../Serial/SerialController.h"
#include "HomeAutomationController.h"

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

#define DEFAULT_BUFLEN 512
int HomeAutomationController::ActivateGarageNetwork(string& resultStatus)
{
	//----------------------
	// Declare and initialize variables.
	int iResult;
	WSADATA wsaData;

	SOCKET ConnectSocket = INVALID_SOCKET;
	struct sockaddr_in clientService;

	int recvbuflen = DEFAULT_BUFLEN;
	char *sendbuf = "Client: sending data test";
	char recvbuf[DEFAULT_BUFLEN] = "";

	//----------------------
	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != NO_ERROR) {
		wprintf(L"WSAStartup failed with error: %d\n", iResult);
		return 1;
	}

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
		WSACleanup();
#endif
		return -1;
	}

	cout << "sending.." << endl;
	//----------------------
	// The sockaddr_in structure specifies the address family,
	// IP address, and port of the server to be connected to.
	clientService.sin_family = AF_INET;
	clientService.sin_addr.s_addr = inet_addr("127.0.0.1");
	clientService.sin_port = htons(8891);

	//----------------------
	// Connect to server.
	iResult = connect(ConnectSocket, (SOCKADDR*)&clientService, sizeof(clientService));
	if (iResult == SOCKET_ERROR) {
		cout << "connect failed with error:  " << endl << WSAGetLastError();
#ifdef _WIN32
		closesocket(ConnectSocket);
		WSACleanup();
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
		cout << "send failed with error: " << WSAGetLastError() << endl;
#ifdef _WIN32
		closesocket(ConnectSocket);
		WSACleanup();
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
		WSACleanup();
#else
		close(ConnectSocket)
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
		WSACleanup();
		return -1;
	}
#else
	close(ConnectSocket)
#endif

#ifdef _WIN32
	WSACleanup();
#endif
	return 0;
}

HomeAutomationController* HomeAutomationController::Create()
{
	static HomeAutomationController singletonController;
	return &singletonController;
}
