#include <iostream>
#include "HttpServer.h"
#include "../Serial/SerialHandler.h"
#include "../trunk/src/hl_sha1wrapper.h"

static const std::string base64_chars =
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"abcdefghijklmnopqrstuvwxyz"
"0123456789+/";

static const std::string webSocketHeader = "HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Accept: ";

void HttpServer::initializeWebSocketConnection(int clientSocket, const std::string& request) const
{
	using namespace std;

	const size_t keyPosition = request.find("Sec-WebSocket-Key: ");
	string keyString;
	if (keyPosition != string::npos)
	{
		const size_t keyPositionEnd = request.find_first_of('\r', keyPosition + 19);
		if (keyPositionEnd == request.npos) return;
		keyString = request.substr(keyPosition + 19, keyPositionEnd - keyPosition - 19);
	}

	//Perform the initial connection handshake:
	//Concatenate the client's key with the standard key and get the SHA1 hash
	const string keyCat = keyString + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
	hashwrapper* myWrapper = new sha1wrapper();
	const string hashedKey = myWrapper->getHashFromString(keyCat);
	size_t outputLength = 0;
	unsigned char inp[200];
	int inpLen = 0;
	size_t ind = (hashedKey.size() - 1) / 2;

	for (int a = 0; a < hashedKey.size(); ++a)
	{
		if (hashedKey[hashedKey.size() - a - 1] >= 97)
		{
			if (a % 2 == 0)
			{
				inp[ind] = hashedKey[hashedKey.size() - a - 1] - 97 + 10;
			}
			else
			{
				int newChar = (hashedKey[hashedKey.size() - a - 1] - 97 + 10) << 4;
				inp[ind] += (hashedKey[hashedKey.size() - a - 1] - 97 + 10) << 4;
			}
		}
		else
		{
			if (a % 2 == 0)
			{
				inp[ind] = hashedKey[hashedKey.size() - a - 1] - 48;
			}
			else
			{
				inp[ind] += (hashedKey[hashedKey.size() - a - 1] - 48) << 4;
			}
		}
		if (a % 2 == 1)
		{
			--ind;
			++inpLen;
		}
	}

	//We need to convert the resulting SHA1 hash to Base64
	int co = 0;
	int byteCounter = 0;
	string finalString = "";
	const int lengthRemainder = (hashedKey.size()) % 3;
	if (lengthRemainder != 0)
	{
		inp[inpLen] = 0;
		if (lengthRemainder == 2)
		{
			inp[inpLen + 1] = 0;
		}
	}
	for (co = inpLen - 1 + lengthRemainder; co >= 0;)
	{
		int nextDigit = 0;
		if (byteCounter == 0 || byteCounter % 8 == 0)
		{
			const unsigned char firstHalf = inp[co] & 0x3F;
			nextDigit = firstHalf;
		}
		else if (byteCounter % 8 == 6)
		{
			const unsigned char firstHalf = inp[co] & 0xC0;
			const unsigned char secondHalf = inp[co - 1] & 0x0F;
			nextDigit = (firstHalf >> 6) + (secondHalf << 2);
			--co;
		}
		else if (byteCounter % 8 == 4)
		{
			const unsigned char firstHalf = inp[co] & 0xF0;
			const unsigned char secondHalf = inp[co - 1] & 0x03;
			nextDigit = (firstHalf >> 4) + (secondHalf << 4);
			--co;
		}
		else if (byteCounter % 8 == 2)
		{
			const unsigned char firstHalf = inp[co] & 0xFC;
			nextDigit = (firstHalf >> 2);
			--co;
		}

		finalString = base64_chars[nextDigit] + finalString;
		byteCounter += 6;
	}
	for (int a = 0; a < lengthRemainder; ++a)
	{
		finalString[finalString.size() - 1 - a] = '=';
	}

	const string returnHeader = webSocketHeader + finalString + "\r\n\r\n";
	const char* const finalHeader = returnHeader.c_str();
	cout << finalHeader << endl;
	const size_t iSendResult2 = send(clientSocket, finalHeader, sizeof(char) * returnHeader.size(), 0);
}

//for sending messages whose length can be described with one byte
void HttpServer::sendWebSocketMessageShort(int clientSocket, const std::string& message) const
{
	using namespace std; 
	char* fullMessage = new char[message.size() + 2];
	fullMessage[0] = 129;
	fullMessage[1] = static_cast<char>(message.size());
	memcpy(fullMessage + 2, message.c_str(), message.size());
	const size_t iSendResult = send(clientSocket, fullMessage, fullMessage[1] + 2, 0);

	delete[] fullMessage;
}

void HttpServer::maintainWebSocketConnection(int clientSocket) const
{
	using namespace std;

	char bufferRcv[1024];
	int cc = 0;
	bool hasSent = false;
	while (true)
	{
		int recvLen = -1;
		while (recvLen < 0)
		{
			fd_set readSet;
			FD_ZERO(&readSet);
			FD_SET(clientSocket, &readSet);
			timeval timeout;
			timeout.tv_sec = 0;  // Zero timeout (poll)
			timeout.tv_usec = 0;
			if (select(clientSocket, &readSet, NULL, NULL, &timeout) == 1)
			{
				recvLen = recv(clientSocket, bufferRcv, 1024, 0);
			}
		}

		bufferRcv[recvLen] = 0;
		cout << "Websocket Message Recieved: " << recvLen << " bytes: " << bufferRcv << endl;
		unsigned char messageType;
		unsigned char messageLength;
		unsigned char mask[4];
		memcpy(&messageType, bufferRcv, sizeof(unsigned char));
		memcpy(&messageLength, bufferRcv + 1, sizeof(unsigned char));
		memcpy(&mask, bufferRcv + 2, sizeof(unsigned char));
		memcpy((mask + 1), bufferRcv + 3, sizeof(unsigned char));
		memcpy((mask + 2), bufferRcv + 4, sizeof(unsigned char));
		memcpy((mask + 3), bufferRcv + 5, sizeof(unsigned char));
		//data at ind 6
		cout << "Message Type: " << (unsigned int) (messageType) << endl;
		cout << "Message Length: " << (unsigned int) (messageLength) << " lengthCheck 1: " << (unsigned int) (messageLength & 0x7E) <<
			" lengthCheck 2: " << (unsigned int) (messageLength & 0x7F) << endl;

		messageLength = (messageLength & 0x7F);
		cout << "True Message Length: " << (unsigned int) messageLength << endl;
		//cout << "Mask: " << mask << endl;

		string decodedMessage;
		for (int i = 6, j = 0; i < recvLen; ++i, ++j)
		{
			unsigned char unmaskedChar = ((unsigned char) (bufferRcv[i]) ^ mask[j % 4]);
			decodedMessage += unmaskedChar;
		}

		cout << "Decoded Message: " << decodedMessage << endl;
		recvLen = -1;
		/*auto potentialCallback = this->webSocketMessageCallbacks.find(decodedMessage);
		
		if (potentialCallback != this->webSocketMessageCallbacks.end())
		{
			//this->webSocketMessageCallbacks[decod
			potentialCallback->second();
		}*/
		if (decodedMessage == "hello")
		{

		}
		else if (decodedMessage == "initserial")
		{
			cout << "Initializing serial connection..";
			if (InitializeSerialConnection())
			{
				cout << "..Successful" << endl;
			}
			else
			{
				cout << "..FAILED" << endl;
			}
		}
		else if (decodedMessage == "turnon")
		{
			cout << "Sending RED LED ON command to serial..";
			char buf[3] = "on";

			buf[2] = '\n';
			if (WriteData(buf, 3))
			{
				cout << "..Successful" << endl;
			}
			else
			{
				cout << "..FAILED" << endl;
			}
		}
		else if (decodedMessage == "turnoff")
		{
			cout << "Sending RED LED OFF command to serial..";
			char buf[4] = "off";

			buf[3] = '\n';
			if (WriteData(buf, 4))
			{
				cout << "..Successful" << endl;
			}
			else
			{
				cout << "..FAILED" << endl;
			}
		}
		else if (decodedMessage == "testrequest")
		{
			cout << "Received server test request.  Sending response." << endl;
			this->sendWebSocketMessageShort(clientSocket, "The Server Apparently Works");
		}
		else if (decodedMessage == "turnleft")
		{
			cout << "Sending TURN LEFT motor command to serial...";
			char buf[9] = "turnleft";
			buf[8] = '\n';
			if (WriteData(buf, 9))
			{
				cout << "..Successful" << endl;
			}
			else
			{
				cout << "..FAILED" << endl;
			}
		}
		else if (decodedMessage == "turnright")
		{
			cout << "Sending TURN RIGHT motor command to serial...";
			char buf[10] = "turnright";
			buf[9] = '\n';
			if (WriteData(buf, 10))
			{
				cout << "..Successful" << endl;
			}
			else
			{
				cout << "..FAILED" << endl;
			}
		}
		else if (decodedMessage == "toggle")
		{
			cout << "Sending TOGGLE command to serial...";
			char buf[7] = "toggle";
			buf[6] = '\n';
			if (WriteData(buf, 7))
			{
				cout << "..Successful" << endl;
			}
			else
			{
				cout << "..FAILED" << endl;
			}
		}
		cout << endl;
	}
}