#include "Util.h"
#include <time.h>

using namespace std;

namespace Util
{
	void to_upper(wstring& s)
	{
		for (size_t a = 0; a < s.length(); ++a)
		{
			s[a] = toupper(s[a]);
		}
	}

	wchar_t* to_upper(wchar_t* s, size_t len)
	{
		for (size_t a = 0; a < len; ++a)
		{
			s[a] = toupper(s[a]);
		}

		return s;
	}

	void to_upper(string& s)
	{
		for (size_t a = 0; a < s.length(); ++a)
		{
			s[a] = toupper(s[a]);
		}
	}

	char* to_upper(char* s, size_t len)
	{
		for (size_t a = 0; a < len; ++a)
		{
			s[a] = toupper(s[a]);
		}

		return s;
	}
	
	void to_lower(string& s)
	{
		for (size_t a = 0; a < s.length(); ++a)
		{
			s[a] = tolower(s[a]);
		}
	}

	vector<string> split(const string& input, const string& delimString)
	{
		size_t currentIndex = 0;
		vector<string> results;
		size_t nextIndex = 0;
		while (nextIndex != string::npos && currentIndex < input.size())
		{
			nextIndex = input.find(delimString, currentIndex);

			results.push_back(input.substr(currentIndex, nextIndex - currentIndex));
			currentIndex = nextIndex + delimString.length();
		}

		return results;
	}
	
	string safe_substr(const string& input, const size_t pos, const size_t len)
	{
		if (pos >= input.length())
		{
			return "";
		}

		return input.substr(pos, len);
	}

	//copied from stack overflow; have yet to read the code inside this function. seems to work anyway.
	string CurrentDateTime() 
	{
		time_t     now = time(0);
		struct tm  tstruct;
		char       buf[80];
#ifdef _WIN32
		localtime_s(&tstruct, &now);
#else
		localtime_r(&now, &tstruct);
#endif
		strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);

		return buf;
	}
}