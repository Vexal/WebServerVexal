#pragma once
#include <functional>
#include <unordered_map>
#include "../HttpServer/HttpRequest.h"

class Page;
class Folder;

class PageConstructor
{
private:
public:
	//caller is responsible for freeing memory used by constructed page.
	//check for param in param list first; then check function param
	static Page const* ConstructPage(const Page* const page,
		const Folder* const root,
		const HttpRequest& request,
		const std::unordered_map<std::string, std::string>& params = {},
		const std::unordered_map<std::string, std::function<std::string(std::string const&)> > paramFuncs = {});
};