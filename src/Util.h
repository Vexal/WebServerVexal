#pragma once
#include <string>
#include <vector>
#include <unordered_map>

namespace Util
{
	void to_upper(std::wstring& s);
	wchar_t* to_upper(wchar_t* s, size_t len);
	void to_upper(std::string& s);
	char* to_upper(char* s, size_t len);
	void to_lower(std::string& s);
	std::vector<std::string> split(const std::string& input, const std::string& delimString);
	std::string safe_substr(const std::string& input, const size_t pos, const size_t len = std::string::npos);
	//returns a default value but does not insert new entry if not found.
	template<typename K, typename V> V get_with_default(const std::unordered_map<K, V>& m, const K& key, const V& defaultReturn)
	{
		using namespace std;
		const auto it = m.find(key);
		if (it == m.end())
		{
			return defaultReturn;
		}

		return it->second;
	}

	std::string CurrentDateTime();
	unsigned long long EpochTimeMillis();
	std::string GetBase64String(const unsigned char* const inp, const unsigned int len);

	const std::string base64_chars =
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz"
		"0123456789+/";
}