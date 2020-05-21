#pragma once
#include <vector>
#include "../jsoncpp/include/json/json.h"
#include "../Utils/CprHelper.h"
#include "../Utils/S2W.h"
#include "../Utils/Base64.h"
#include "../UIAdapter/HomeItem.h"
#include "../Utils/PathHelper.h"
#include "../Utils/CodeCrc32.h"
#include "DBHelper.h"
class ServerAPI_Home {
	static inline std::wstring Url(std::wstring part) {
		return ServerIPAPI + part;
	}
	static inline std::wstring Header(std::wstring part = L"") {
		
		return L"";
	}

	static wstring HomeItemListFromJson(string body, vector<HomeItem>& ChildList) {
		return L"网络错误";
	}
	static wstring _GetDat(wstring url, uint32_t crc32) {
		
		return L"success";
	}
public:
	static std::wstring SubmiteSearch(std::wstring key, vector<HomeItem>& ChildList) {
		
		return L"网络错误";
	}

	//检测新版本，返回新版本号(ver1.0.27.131)或者返回error
	static std::wstring CheckUP() {
		
		return L"error";
	}

	static std::wstring PostLog(string& buffer, size_t len) {
		
		return L"error";
	}
};