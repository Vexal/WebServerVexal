#pragma once
#include <string>

enum NODE_TYPE
{
	UNDEFINED,
	PAGE,
	FOLDER
};
class WebNode
{
protected:
#ifdef WIN32
	std::string name;
	std::string fullPath;
#else
	std::string name;
	std::string fullPath;
#endif
	NODE_TYPE nodeType;

public:
#ifdef WIN32
	WebNode(const std::string& fullPath, const std::string& name, NODE_TYPE);
#else
	WebNode(const std::string& fullPath, const std::string& name, NODE_TYPE);
#endif
	NODE_TYPE GetNodeType() const {return this->nodeType;}
#ifdef WIN32
	std::string GetName() const {return this->name;}
#else
	std::string GetName() const {return this->name;}
#endif

	std::string GetFullPath() const { return this->fullPath; }
	virtual ~WebNode();
};