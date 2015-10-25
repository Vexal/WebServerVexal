#include "ContentHost.h"
#include "Folder.h"

ContentHost::ContentHost(const std::string& rootFolder) :
	rootFolder(rootFolder),
	rootDirectory(new Folder(this->rootFolder + "/", this->rootFolder))
{
	
}