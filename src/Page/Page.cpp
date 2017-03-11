#include <string>
#include <fstream>
#include <iostream>

#include "Page.h"
#include <sstream>
#ifndef _WIN32
#include <string.h>
#endif

using namespace std;

Page::Page(const string& filePath, const string& name) :
	WebNode(filePath, name, PAGE)
{
	size_t dotInd = this->name.find_first_of('.');
	string command(this->name, dotInd + 1, this->name.length() - dotInd);
	if(command.compare("html") == 0 || command.compare("htm") == 0 || command.compare("txt") == 0)
	{
		this->pageType = HTML;
		this->contentType = TEXT;
	}
	else if(command.compare("png") == 0)
	{
		this->pageType = PNG;
		this->contentType = IMAGE;
	}
	else if(command.compare("jpg") == 0 || command.compare("jpeg") == 0)
	{
		this->pageType = JPG;
		this->contentType = IMAGE;
	}
	else if(command.compare("ico") == 0)
	{
		this->pageType = ICO;
		this->contentType = IMAGE;
	}
	else if(command.compare("gif") == 0)
	{
		this->pageType = GIF;
		this->contentType = IMAGE;
	}
	else if(command.compare("css") == 0)
	{
		this->pageType = CSS;
		this->contentType = TEXT;
	}
	else if(command.compare("wav") == 0)
	{
		this->pageType = WAV;
		this->contentType = SOUND;
	}
	else if(command.compare("mp3") == 0)
	{
		this->pageType = MP3;
		this->contentType = SOUND;
	}

	this->loadContent();
}

Page::Page(const string& filePath, const string& name, const string& text) :
WebNode(filePath, name, PAGE)
{
	this->content = new char[text.length() + 1];
	memcpy(this->content, text.c_str(), text.length());
	this->content[text.length()] = 0;
	this->pageType = HTML;
	this->contentType = TEXT;
	this->valid = true;

	this->contentLength = text.length();
}

void Page::loadContent()
{
	switch(this->contentType)
	{
	case TEXT:
		{
			ifstream myFile(this->fullPath, ios::ate);
			if(myFile.is_open())
			{
				myFile.seekg(0, ios::beg);
				stringstream fileStream;
				fileStream << myFile.rdbuf();
				this->contentLength = fileStream.str().length();
				this->content = new char[this->contentLength + 1];

				memcpy(this->content, fileStream.str().c_str(), sizeof(char) * this->contentLength);
				this->content[this->contentLength] = 0;
				myFile.close();
			}
			else
			{
				cout << "Failed to load page: " << this->fullPath << endl;
				this->valid = false;
			}
		}
		break;
	case IMAGE:
		{
			ifstream myFile(this->fullPath, ios::in | ios::binary | ios::ate);
			if(myFile.is_open())
			{
				this->contentLength = myFile.tellg();
				myFile.seekg(0, ios::beg);
				this->content = new char[this->contentLength + 1];
				myFile.read(this->content, this->contentLength);
				myFile.close();
				this->valid = true;			
			}
			else
			{
				cout << "Failed to load image: " << this->fullPath << endl;
				this->valid = false;
			}
		}
		break;
	case SOUND:
		{
			ifstream myFile(this->fullPath, ios::in | ios::binary | ios::ate);
			if(myFile.is_open())
			{
				this->contentLength = myFile.tellg();
				myFile.seekg(0, ios::beg);
				this->content = new char[this->contentLength + 1];
				myFile.read(this->content, this->contentLength);
				myFile.close();
				this->valid = true;
				//cout << "Loaded sound: " << this->fullPath << " with size " << this->contentLength << endl;
			}
			else
			{
				cout << "Failed to load sound: " << this->fullPath << endl;
				this->valid = false;
			}
		}
		break;
	}
}

Page::~Page()
{
	delete[] this->content;
}
