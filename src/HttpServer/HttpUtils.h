#pragma once
#include <string>
#include <unordered_map>

namespace HttpUtils
{
	const std::string REQUEST_METHOD_FIELD = ":method";
	const std::string REQUEST_URI_FIELD = ":request-uri";
	const std::string REQUEST_HTTP_VERSION_FIELD = ":http-version";
	const std::string MALFORMED_REQUEST_FIELD = ":MALFORMED-REQUEST";

	enum class HttpRequestTypes
	{
		INVALID,
		GET,
		POST
	};

	std::string urlDecode(const std::string& s, bool plussesAreSpaces = true);
	std::string urlEncode(const std::string& s, bool plussesAreSpaces = true);
	std::string entityEncode(const std::string& s);
	std::unordered_map<std::string, std::string> GetHttpGetParameters(const std::string& uriAfterQuestionMark);
	std::unordered_map<std::string, std::string> GetHttpHeaders(const std::string& request);
	std::string GetDomain(const std::string& url);
	HttpRequestTypes GetHttpRequestTypeFromString(const std::string& requestWord);
	HttpRequestTypes GetHttpRequestType(const std::string& request);

	const std::unordered_map<char, std::string> urlEncodeMap = {
		{ ' ', "%20" },{ '!', "%21" },{ '\"', "%22" },{ '#', "%23" },{ '$', "%24" },{ '%', "%25" },{ '&', "%26" },
		{ '\'', "%27" },{ '(', "%28" },{ ')"', "%29" },{ '*', "%2A" },{ '+', "%2B" },{ ',', "%2C" },{ '-', "%2D" },
		{ '.', "%2E" },{ '/', "%2F" },{ '\"', "%22" },{ '#', "%23" },{ '$', "%24" },{ '%', "%25" },{ '&', "%26" },
	};
	const std::unordered_map<char, std::string> entityEncodeMap = {
		{ '<', "&lt;" },{ '>', "&gt;" },{ '&', "&amp"}
	};
}