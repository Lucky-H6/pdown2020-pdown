#pragma once
#include <vector>
#include "../jsoncpp/include/json/json.h"
#include "../Utils/CprHelper.h"
#include "../Utils/S2W.h"
#include "../Utils/Base64.h"
#include "DBHelper.h"
class ServerAPI_User {
	static inline std::wstring Url(std::wstring part) {
		return ServerIPAPI + part;
	}
	static inline std::wstring Header(std::wstring part = L"") {
		return L"";
	}
public:
	static std::wstring SendCode(std::wstring phone) {
		
		return L"ÍøÂç´íÎó";
	}
	static std::wstring Login(std::wstring phone, std::wstring code, std::wstring& data) {
		
		return L"ÍøÂç´íÎó";
	}



};