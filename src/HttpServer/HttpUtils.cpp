#include "HttpUtils.h"
#include "../Util.h"

using namespace std;
namespace HttpUtils
{
	unordered_map<string, string> GetHttpGetParameters(const string& uriAfterQuestionMark)
	{
		unordered_map<string, string> paramMap;
		size_t currentInd = 0;

		do 
		{
			if (currentInd > uriAfterQuestionMark.length())
				break;

			const size_t nextInd = uriAfterQuestionMark.find_first_of('&', currentInd);

			const string paramAndValue = uriAfterQuestionMark.substr(currentInd, nextInd - currentInd);
			const size_t equals = paramAndValue.find_first_of('=');
			const string param = paramAndValue.substr(0, equals);
			const string value = Util::safe_substr(paramAndValue, equals + 1);

			if (!param.empty() && !value.empty())
			{
				paramMap[param] = value;
			}

			currentInd = nextInd;
		} while (currentInd++ != string::npos);

		return paramMap;
	}

	unordered_map<string, string> GetHttpHeaders(const string& request)
	{
		unordered_map<string, string> results;
		const vector<string> headerFields = Util::split(request, "\r\n");
		if (!headerFields.empty())
		{
			const vector<string> requestInfo = Util::split(headerFields[0], " ");
			if (requestInfo.size() != 3)
			{
				results[MALFORMED_REQUEST_FIELD] = request;
				return results;
			}

			results[REQUEST_METHOD_FIELD] = requestInfo[0];
			results[REQUEST_URI_FIELD] = requestInfo[1];
			results[REQUEST_HTTP_VERSION_FIELD] = requestInfo[2];

			for (int i = 1; i < headerFields.size(); ++i) // skip requestInfo line
			{
				size_t colonIndex = headerFields[i].find(':');
				if (colonIndex > headerFields[i].length())
				{
					//results.clear();
					//results[MALFORMED_REQUEST_FIELD] = request;
					//return results;
					continue;
				}

				string fieldName = headerFields[i].substr(0, colonIndex);
				Util::to_lower(fieldName);

				//pass colon and white space
				while (colonIndex < headerFields[i].length() && (headerFields[i][colonIndex] == ':' || headerFields[i][colonIndex] == ' '))
					++colonIndex;

				results[fieldName] = Util::safe_substr(headerFields[i], colonIndex);
			}
		}

		return results;
	}

	string GetDomain(const string& url)
	{
		size_t httpEnd = url.find("http://");
		if (httpEnd == string::npos)
		{
			httpEnd = url.find("https://");
			if (httpEnd != string::npos)
			{
				httpEnd = 8;
			}
		}
		else
		{
			httpEnd = 7;
		}

		if (httpEnd == string::npos)
		{
			httpEnd = 0;
		}

		const size_t domainEnd = url.find('/', httpEnd);
		return url.substr(httpEnd, domainEnd - httpEnd);
	}

	HttpRequestTypes GetHttpRequestType(const string& request)
	{
		const string firstWord = request.substr(0, request.find_first_of(' '));
		return GetHttpRequestTypeFromString(firstWord);
	}

	HttpRequestTypes GetHttpRequestTypeFromString(const string& requestWord)
	{
		return HttpRequestTypes::GET;
		if (requestWord == "GET")
		{
			return HttpRequestTypes::GET;
		}
		if (requestWord == "POST")
		{
			return HttpRequestTypes::POST;
		}

		return HttpRequestTypes::INVALID;
	}

	string urlDecode(const string& s, bool plussesAreSpaces)
	{
		string newString;
		newString.reserve(s.length());

		for (int a = 0; a < s.length(); ++a)
		{
			if (s[a] == 0)
			{
				break;
			}
			if (s[a] == '+' && plussesAreSpaces)
			{
				newString.append(" ");
			}
			else if (s[a] == '%' && a + 2 < s.length())
			{
				if (s[a + 2] == 'A' && s[a + 1] == '0')
				{
					newString.append({ 10 });
				}
				else if (s[a + 2] == 'A' && s[a + 1] == '3')
				{
					newString.append(":");
				}
				else if (s[a + 2] == 'F' && s[a + 1] == '2')
				{
					newString.append("/");
				}
				else if (s[a + 2] == '0' && s[a + 1] == '2')
				{
					newString.append(" ");
				}
				a += 2;
			}
			else
			{
				newString.append({ s[a] });
			}
		}

		return newString;
	}

	string urlEncode(const string& s, bool plussesAreSpaces)
	{
		string newString;
		newString.reserve(s.length());

		for (int a = 0; a < s.size(); ++a)
		{
			if (s[a] == 0)
			{
				break;
			}

			if (s[a] >= '0' && s[a] <= '9' ||
				s[a] >= 'A' && s[a] <= 'Z' ||
				s[a] >= 'a' && s[a] <= 'z' ||
				s[a] == '-' || s[a] == '_' ||
				s[a] == '.' || s[a] == '~')
			{
				newString += s[a];
			}
			else
			{
				const auto enc = urlEncodeMap.find(s[a]);
				if (enc != urlEncodeMap.end())
				{
					newString += enc->second;
				}
			}
		}

		return newString;
	}

	string entityEncode(const string& s)
	{
		string newString;
		newString.reserve(s.length());

		for (int a = 0; a < s.size(); ++a)
		{
			if (s[a] == 0)
			{
				break;
			}

			const auto enc = entityEncodeMap.find(s[a]);
			if (enc != entityEncodeMap.end())
			{
				newString += enc->second;
			}
			else
			{
				newString += s[a];
			}
		}

		return newString;
	}
}
