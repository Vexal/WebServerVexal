#pragma once
#include <unordered_map>

class Page;
class Folder;

class PageConstructor
{

public:
	//caller is responsible for freeing memory used by constructed page.
	static Page const* ConstructPage(const Page* const page, const Folder* const root, const std::unordered_map<std::string, std::string> params = {});
};