#pragma once

bool WriteData(const char *buffer, unsigned int nbChar);
int ReadData(char *buffer, unsigned int nbChar);
bool InitializeSerialConnection();
void ListenSerial();