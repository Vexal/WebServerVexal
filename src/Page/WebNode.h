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
	std::string name;
	std::string fullPath;
	NODE_TYPE nodeType;

public:
	WebNode(const std::string& fullPath, const std::string& name, NODE_TYPE);
	NODE_TYPE GetNodeType() const {return this->nodeType;}
	std::string GetName() const {return this->name;}

	std::string GetFullPath() const { return this->fullPath; }
	virtual ~WebNode();
};