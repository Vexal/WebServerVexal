#include "Util.h"
#include <time.h>
#include <chrono>
#include <algorithm>

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

	unsigned long long EpochTimeMillis()
	{
		using namespace std::chrono;
		return duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count();
	}

	//probably an off by 2 error somewhere.  why did I do this myself.
	string GetBase64String(const unsigned char* const inp, const unsigned int len)
	{
		int co = 0;
		int byteCounter = 0;
		string finalString = "";
		finalString.reserve(len);
		const int lengthRemainder = (len * 8) % 3;

		for (co = len - 1 + lengthRemainder; co >= 0;)
		{
			int nextDigit = 0;
			if (byteCounter == 0 || byteCounter % 8 == 0)
			{
				const unsigned char firstHalf = co >= len ? 0 : inp[co] & 0x3F;
				nextDigit = firstHalf;
			}
			else if (byteCounter % 8 == 6)
			{
				const unsigned char firstHalf = co >= len ? 0 : inp[co] & 0xC0;
				const unsigned char secondHalf = (co - 1) >= len ? 0 : inp[co - 1] & 0x0F;
				nextDigit = (firstHalf >> 6) + (secondHalf << 2);
				--co;
			}
			else if (byteCounter % 8 == 4)
			{
				const unsigned char firstHalf = co >= len ? 0 : inp[co] & 0xF0;
				const unsigned char secondHalf = (co - 1) >= len ? 0 : inp[co - 1] & 0x03;
				nextDigit = (firstHalf >> 4) + (secondHalf << 4);
				--co;
			}
			else if (byteCounter % 8 == 2)
			{
				const unsigned char firstHalf = co >= len ? 0 : inp[co] & 0xFC;
				nextDigit = (firstHalf >> 2);
				--co;
			}

			finalString += base64_chars[nextDigit];
			byteCounter += 6;
		}

		for (int a = 0; a < lengthRemainder; ++a)
		{
			finalString[a] = '=';
		}

		reverse(finalString.begin(), finalString.end());
		return finalString;
	}
}