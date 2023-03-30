#pragma once
#include <string>
#include <vector>

// このクラスは dll からエクスポートされました
class CStringEx {

public:
	template <typename ... Args>
	static std::string Format(const std::string& fmt, Args ... args)
	{
		size_t len = snprintf(nullptr, 0, fmt.c_str(), args ...);
		std::vector<char> buf(len + 1);
		snprintf(&buf[0], len + 1, fmt.c_str(), args ...);
		return std::string(&buf[0], &buf[0] + len);
	};

	static std::string ltrim(const std::string& s)
	{
		size_t start = s.find_first_not_of(" \n\r\t\f\v");
		return (start == std::string::npos) ? "" : s.substr(start);
	};
	static std::string rtrim(const std::string& s)
	{
		size_t end = s.find_last_not_of(" \n\r\t\f\v");
		return (end == std::string::npos) ? "" : s.substr(0, end + 1);
	};
	static std::string Trim(const std::string& s)
	{
		return rtrim(ltrim(s));
	};

	// WCHAR
	template <typename ... Args>
	static std::wstring Format(const std::wstring& fmt, Args ... args)
	{
		size_t len = swprintf(nullptr, 0, fmt.c_str(), args ...);
		std::vector<WCHAR> buf(len + 1);
		swprintf(&buf[0], len + 1, fmt.c_str(), args ...);
		return std::wstring(&buf[0], &buf[0] + len);
	};

	static std::wstring ltrim(const std::wstring& s)
	{
		size_t start = s.find_first_not_of(L" \n\r\t\f\v");
		return (start == std::string::npos) ? L"" : s.substr(start);
	};
	static std::wstring rtrim(const std::wstring& s)
	{
		size_t end = s.find_last_not_of(L" \n\r\t\f\v");
		return (end == std::wstring::npos) ? L"" : s.substr(0, end + 1);
	};
	static std::wstring Trim(const std::wstring& s)
	{
		return rtrim(ltrim(s));
	};

	// wstring --> string
	static std::string ToString(const std::wstring& text)
	{
		size_t size;
		char* cBuf = new char[text.size() * MB_CUR_MAX + 1];
		wcstombs_s(&size, cBuf, text.size() * MB_CUR_MAX + 1, text.c_str(), _TRUNCATE);
		std::string str = cBuf;
		delete[] cBuf;
		return str;
	}

	// string --> wstring
	static std::wstring ToWString(const std::string& text)
	{
		size_t size;
		wchar_t* wcBuf = new wchar_t[text.size() + 1];
		mbstowcs_s(&size, wcBuf, text.size() + 1, text.c_str(), _TRUNCATE);
		std::wstring wstr = wcBuf;
		delete[] wcBuf;
		return wstr;
	}

	// split
	static std::vector<std::string> split(std::string str, char del) {
		size_t first = 0;
		size_t last = str.find_first_of(del);

		std::vector<std::string> result;

		while (first < str.size()) {
			std::string subStr(str, first, last - first);

			result.push_back(subStr);

			first = last + 1;
			last = str.find_first_of(del, first);

			if (last == std::string::npos) {
				last = str.size();
			}
		}
		return result;
	}

};
