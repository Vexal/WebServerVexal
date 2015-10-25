#pragma once
#include <string>

void to_upper(std::wstring& s);
wchar_t* to_upper(wchar_t* s, size_t len);
void to_upper(std::string& s);
char* to_upper(char* s, size_t len);