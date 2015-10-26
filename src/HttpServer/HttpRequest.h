#pragma once
#include <unordered_map>
#include <string>

struct HttpRequest
{
	const std::string request;
	const std::unordered_map<std::string, std::string> paramMap;
	const std::unordered_map<std::string, std::string> headers;
	const std::string requestTarget;
	const std::string referDomain;
	const std::string clientAddressString;

	std::string GetParameter(const std::string& param, const std::string& defaultResult = "") const;
};