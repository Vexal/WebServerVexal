#include "Folder.h"
#include <string>
#include <iostream>
#ifdef WIN32
#include <windows.h>
#include <shlwapi.h>
#include <atldbcli.h>
#else
#include <dirent.h>
#include <cstring>
#endif
#include "Page.h"
#include "Util.h"

using namespace std;
extern bool printEverything;

Folder::Folder(const string& fullPath, const string& name) :
	WebNode(fullPath, name, FOLDER)
{
	if(this->fullPath == "Content/")
	{
		this->error404Page = new Page(this->fullPath + "404.html", "/404.html");
		if (this->error404Page->IsValid())
		{
			this->pages["404.HTML"] = this->error404Page;
		}

		this->defaultPage = new Page(this->fullPath + "Home.html", "/HOME.html");
		if (this->defaultPage->IsValid())
		{
			this->pages["HOME.HTML"] = this->defaultPage;
		}
	}

#ifdef _WIN32
	WIN32_FIND_DATA data;
	const string path(this->fullPath + "*");
	wchar_t* wBuffer = new wchar_t[path.size() + 2];
	size_t numConverted;
	mbstowcs_s(&numConverted, wBuffer, path.size() + 1, path.c_str(), path.size() + 1);
	const HANDLE hFile = FindFirstFile(wBuffer, &data);

	if  (hFile == INVALID_HANDLE_VALUE)
		wcout << L"invalid file hfile";

	while(FindNextFile(hFile, &data) != 0 || GetLastError() != ERROR_NO_MORE_FILES)
	{
		if(data.cFileName[0] != L'.')
		{
			if(FILE_ATTRIBUTE_DIRECTORY & data.dwFileAttributes)
			{
				//wcout << L"Found directory: " << data.cFileName << endl;

				size_t convnum;
				char* pathChar = new char[strlenT(data.cFileName) + 1];
				wcstombs_s(&convnum, pathChar, strlenT(data.cFileName) + 1, data.cFileName, strlenT(data.cFileName) + 1);
				string s(pathChar);
				this->pages[to_upper(pathChar, strlenT(data.cFileName))] = new Folder(this->fullPath + s + "/", s);
				delete[] pathChar;
			}
			else
			{
				size_t convnum;
				char* pathChar = new char[strlenT(data.cFileName) + 1];
				wcstombs_s(&convnum, pathChar, strlenT(data.cFileName) + 1, data.cFileName, strlenT(data.cFileName) + 1);
				string s(pathChar);
				//wcout << L"Found file: " << data.cFileName << endl;
				this->pages[to_upper(pathChar, strlenT(data.cFileName))] = new Page(this->fullPath + s, s);
				delete[] pathChar;
			}
		}
	}

	delete[] wBuffer;
	FindClose(hFile);
#else
	DIR* d;
	struct dirent *dir;
	string dirPath(this->fullPath + ".");
	d = opendir(dirPath.c_str());
	if (d)
	{
		while ((dir = readdir(d)) != NULL)
		{
			if(dir->d_name[0] == '.')
			{
				continue;
			}
			if(dir->d_type == DT_DIR)
			{
				//cout << "Found directory: " << this->fullPath << dir->d_name << endl;
				this->pages[to_upper(dir->d_name, strlen(dir->d_name))] = new Folder(this->fullPath + dir->d_name + "/", dir->d_name);
			}
			else
			{
				//cout << "Found file: " << dir->d_name << endl;
				this->pages[to_upper(dir->d_name, strlen(dir->d_name))] = new Page(this->fullPath + dir->d_name, dir->d_name);
			}
			//printf("%s\n", dir->d_name);
		}

		closedir(d);
	}
#endif

}

WebNode* Folder::GetPage(const string& pageName) const
{
	const size_t slashInd = pageName.find_first_of('/', 1);
	
	if(slashInd != string::npos)
	{
		const size_t endInd = pageName.length() - slashInd;
		string localName = string(pageName, 1, slashInd - 1);
		const string remainingName = string(pageName, slashInd, endInd);
		to_upper(localName);
		auto potentialPage =  this->pages.find(localName);
		if(potentialPage != this->pages.end())
		{
			switch (potentialPage->second->GetNodeType())
			{
			case FOLDER:
				Folder* nextFolder = static_cast<Folder*>(potentialPage->second);
				WebNode* nextNode = nextFolder->GetPage(remainingName);
				return nextNode;
				break;
			}
		}
		
		cout << "Error:  " << pageName << " not found in directory " << this->fullPath << endl;		
	}
	else
	{
		if(pageName[0] != '/')
		{
			string pageN = pageName;
			to_upper(pageN);
			auto potentialPage =  this->pages.find(pageN);
			if(potentialPage != this->pages.end())
			{
				if(printEverything)
					cout << "Successfully accessed " << pageName << " in directory " << this->fullPath << endl;
				return potentialPage->second;
			}
		}
		else
		{
			string pageN(pageName, 1, pageName.length() - 1);
			to_upper(pageN);
			auto potentialPage =  this->pages.find(pageN);
			if(potentialPage != this->pages.end())
			{
				if(printEverything)
					cout << "Successfully accessed " << pageName << " in directory " << this->fullPath << endl;

				return potentialPage->second;
			}
		}

		cout << "Error:  " << pageName << " not found in directory " << this->fullPath << endl;
	}
	
	return nullptr;
}

Folder::~Folder()
{
	auto iter = this->pages.begin();
	while(iter != this->pages.end())
	{
		delete iter->second;
		iter++;
	}
	
	this->pages.clear();
}
