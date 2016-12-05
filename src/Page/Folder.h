#pragma once
#include <map>
#include "WebNode.h"
#include "../Logger/Logger.h"

struct PageCompare
{
	bool operator() (const std::string& lhs, const std::string& rhs) const
	{
		return lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
	}
};

class Page;

class Folder : public WebNode
{
private:
	std::map<std::string, WebNode*, PageCompare> pages;
	Page* defaultPage = nullptr;
	Page* error404Page = nullptr;
	static Logger log;

public:
	Folder(const std::string& fullPath, const std::string& name);
	~Folder();

	const Page* GetDefaultPage() const {return this->defaultPage;}
	const Page* GetError404Page() const { return this->error404Page; }
	WebNode* GetPage(const std::string& pageName) const;
};