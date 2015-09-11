#pragma once
#include <unordered_map>
#include <string>

struct HttpRequest
{
	const std::string& request;
	const std::unordered_map<std::string, std::string>& paramMap;

	std::string GetParameter(const std::string& param, const std::string& defaultResult = "") const;
};