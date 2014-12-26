#include "Util.h"

using namespace std;

void to_upper(wstring& s)
{
	for(size_t a = 0; a < s.length(); ++a)
	{
		s[a] = toupper(s[a]);
	}
}

wchar_t* to_upper(wchar_t* s, size_t len)
{
	for(size_t a = 0; a < len; ++a)
	{
		s[a] = toupper(s[a]);
	}

	return s;
}

void to_upper(string& s)
{
	for(size_t a = 0; a < s.length(); ++a)
	{
		s[a] = toupper(s[a]);
	}
}

char* to_upper(char* s, size_t len)
{
	for(size_t a = 0; a < len; ++a)
	{
		s[a] = toupper(s[a]);
	}

	return s;
}