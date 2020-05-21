#pragma once
#include <string>
#include <string\strcpcvt.h>
class S2W
{
public:
	
	static std::wstring StringtoWString(const std::string& stdstring, unsigned int code_page= CP_ACP)
	{
		const char* str = stdstring.c_str();
		size_t len = stdstring.length();
		if (len <= 0) return L"";

		std::wstring buf;
		int rc_chars = MultiByteToWideChar(code_page,
			0,
			str,
			len,
			NULL,
			0);

		if (len == -1)
			--rc_chars;
		if (rc_chars == 0)
			return buf;

		buf.resize(rc_chars);
		MultiByteToWideChar(code_page,
			0,
			str,
			len,
			const_cast<wchar_t*>(buf.c_str()),
			rc_chars);

		return buf;
	}


	static std::string WString2String(const std::wstring& wstr, unsigned int code_page = CP_ACP)
	{
		int len = wstr.length();
		if (len <= 0) return "";
		const wchar_t* pStr = wstr.c_str();
		std::string buf;


		if (len < 0 && len != -1)
		{
			return buf;
		}

		// figure out how many narrow characters we are going to get
		int nChars = WideCharToMultiByte(code_page, 0, pStr, len, NULL, 0, NULL, NULL);
		if (len == -1)
			--nChars;
		if (nChars == 0)
			return "";

		// convert the wide string to a narrow string
		// nb: slightly naughty to write directly into the string like this
		buf.resize(nChars);
		WideCharToMultiByte(code_page, 0, pStr, len, const_cast<char*>(buf.c_str()), nChars, NULL, NULL);

		return buf;
	}
};