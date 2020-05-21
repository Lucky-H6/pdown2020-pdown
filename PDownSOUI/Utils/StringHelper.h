#pragma once
#include "../stdafx.h"
#include <string>
#include <sstream>

using namespace std;
static const std::string unchanged("ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	"abcdefghijklmnopqrstuvwxyz"
	"0123456789!'()*-._~");
class StringHelper
{
public:

	template<typename CharType>
	static size_t StringReplaceAllT(const std::basic_string<CharType>& find,
		const std::basic_string<CharType>& replace,
		std::basic_string<CharType>& output)
	{
		size_t find_length = find.size();
		size_t replace_length = replace.size();
		size_t offset = 0, endpos;
		size_t target = 0, found_pos;
		size_t replaced = 0;
		CharType* data_ptr;

		if (find.empty() || output.empty())
			return 0;

		/*
		 * to avoid extra memory reallocating,
		 * we use two passes to finish the task in the case that replace.size() is greater find.size()
		 */

		if (find_length < replace_length)
		{
			/* the first pass, count all available 'find' to be replaced  */
			for (;;)
			{
				offset = output.find(find, offset);
				if (offset == std::basic_string<CharType>::npos)
					break;
				replaced++;
				offset += find_length;
			}

			if (replaced == 0)
				return 0;

			size_t newsize = output.size() + replaced * (replace_length - find_length);

			/* we apply for more memory to hold the content to be replaced */
			endpos = newsize;
			offset = newsize - output.size();
			output.resize(newsize);
			data_ptr = &output[0];

			memmove((void*)(data_ptr + offset),
				(void*)data_ptr,
				(output.size() - offset) * sizeof(CharType));
		}
		else
		{
			endpos = output.size();
			offset = 0;
			data_ptr = const_cast<CharType*>(&output[0]);
		}

		/* the second pass,  the replacement */
		while (offset < endpos)
		{
			found_pos = output.find(find, offset);
			if (found_pos != std::basic_string<CharType>::npos)
			{
				/* move the content between two targets */
				if (target != found_pos)
					memmove((void*)(data_ptr + target),
					(void*)(data_ptr + offset),
						(found_pos - offset) * sizeof(CharType));

				target += found_pos - offset;

				/* replace */
				memcpy(data_ptr + target,
					replace.data(),
					replace_length * sizeof(CharType));

				target += replace_length;
				offset = find_length + found_pos;
				replaced++;
			}
			else
			{
				/* ending work  */
				if (target != offset)
					memcpy((void*)(data_ptr + target),
					(void*)(data_ptr + offset),
						(endpos - offset) * sizeof(CharType));
				break;
			}
		}

		if (replace_length < find_length)
			output.resize(output.size() - replaced * (find_length - replace_length));

		return replaced;
	}

	static size_t StringReplaceAll(const std::string& find, const std::string& replace, std::string& output)
	{
		return StringReplaceAllT<char>(find, replace, output);
	}

	static size_t StringReplaceAll(const std::wstring& find, const std::wstring& replace, std::wstring& output)
	{
		return StringReplaceAllT<wchar_t>(find, replace, output);
	}

	static string SubString(std::string& str, std::string Begin, std::string End) {
		string outstr = "";
		if (str.length() == 0) return outstr;
		auto f = str.find(Begin);
		if (f != string::npos) {
			f += Begin.length();
			outstr = str.substr(f);
			if (End != "") {
				f = outstr.find(End);
				if (f != string::npos) outstr = outstr.substr(0, f);
			}
		}
		return outstr;
	}
	static wstring SubStringW(std::wstring& str, std::wstring Begin, std::wstring End) {
		wstring outstr = L"";
		if (str.length() == 0) return outstr;
		auto f = str.find(Begin);
		if (f != wstring::npos) {
			f += Begin.length();
			outstr = str.substr(f);
			if (End != L"") {
				f = outstr.find(End);
				if (f != wstring::npos) outstr = outstr.substr(0, f);
			}
		}
		return outstr;
	}

	static std::vector<std::wstring> Split(const std::wstring& to_split, wchar_t delimiter) {
		std::vector<std::wstring> tokens;
		std::wstringstream stream(to_split);
		std::wstring item;
		while (std::getline(stream, item, delimiter)) {
			tokens.push_back(item);
		}
		return tokens;
	}

	static std::wstring FormatFileSize(int64_t filesize) {
		if (filesize <= 0) return L"0B";
		wchar_t szText[64];
		double bytes = (double)filesize;
		DWORD cIter = 0;
		std::wstring pszUnits[] = { (L"B"), (L"KB"), (L"MB"), (L"GB"), (L"TB") };
		DWORD cUnits = 5;

		// move from bytes to KB, to MB, to GB and so on diving by 1024
		while (bytes >= 1000 && cIter < (cUnits - 1))
		{
			bytes /= 1024;
			cIter++;
		}
		int slen = swprintf_s(szText, 64, (L"%.2f"), bytes);
		if (slen > 5) slen = 5;
		std::wstring ret = std::wstring(szText, slen) + pszUnits[cIter];
		return ret;
	}
	/*yyyy-MM-dd HH:MM:SS*/
	static std::string FormatJsTimeStr(size_t jstime, bool date) {

		time_t t = jstime;
		struct tm* p;
		p = localtime(&t);
		char s[100];
		int slen = strftime(s, sizeof(s), "%Y-%m-%d %H:%M:%S", p);
		string ret = string(s, slen);
		if (date) ret = ret.substr(0, ret.find(' '));
		return ret;
	}


	static std::wstring FormatBaiFenZhi(double bytes)
	{
		int bits = (int)(bytes * 100);
		if (bits < 0) bits = 0;
		if (bits > 100) bits = 100;
		return to_wstring(bits) + L"%";
	}

	/*格式化文件夹路径，确保最后以\\结尾 UseWin==false==/结尾 */
	static std::wstring FormatDirPath(std::wstring dir, bool UseWin = true) {
		if (UseWin) {
			StringReplaceAll(L"/", L"\\", dir);
			if (dir[dir.length() - 1] != L'\\') dir += L"\\";
		}
		else {
			StringReplaceAll(L"\\", L"/", dir);
			if (dir[dir.length() - 1] != L'/') dir += L"/";
		}
		return dir;
	}

	/*从1970年至今  秒 10位 js时间戳*/
	static size_t GetTimeNow() {
		time_t timer;
		time(&timer);
		size_t t = (size_t)(timer);
		return t;
	}
	/*从1970年至今  毫秒 13位*/
	static int64_t GetTimeNowHaoMiao() {

		time_t timer;
		time(&timer);
		int64_t t = (int64_t)(timer) * 1000 + GetTickCount() % 1000;//13
		return t;

	}
	/*从1970年至今  微秒 16位*/
	static int64_t GetTimeNowWeiMiao() {
		time_t timer;
		time(&timer);
		int64_t t = (int64_t)(timer) * 1000000 + GetTickCount() % 1000000;//16
		return t;

	}


	static inline char CharToHex(uint8_t x)
	{
		return "0123456789ABCDEF"[x];
	}



	static string EncodeUriComponent(std::string& input)
	{
		string output = "";
		for (size_t i = 0; i < input.size(); ++i) {
			if (unchanged.find(input[i]) != string::npos)
			{
				output.push_back(input[i]);
			}
			else {
				output.push_back('%');
				output.push_back("0123456789ABCDEF"[((uint8_t)input[i] >> 4)]);
				output.push_back("0123456789ABCDEF"[((uint8_t)input[i] % 16)]);
			}
		}
		return output;
	}
};