#include "HttpRequest.h"

using namespace std;

string HttpRequest::GetParameter(const string& param, const string& defaultResult) const
{
	const auto paramEntry = this->paramMap.find(param);
	if (paramEntry == this->paramMap.end())
	{
		return defaultResult;
	}

	return paramEntry->second;
}
