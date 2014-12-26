#pragma once
#include <string>

class Folder;

class ContentHost
{
private:
	const std::string rootFolder;
	Folder* const rootDirectory;

public:
	ContentHost(const std::string& rootFolder);
	Folder* GetRootDirectory() const {return this->rootDirectory;}
};