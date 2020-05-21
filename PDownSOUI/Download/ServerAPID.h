#pragma once
#include <vector>
#include "../jsoncpp/include/json/json.h"
#include "../Utils/CprHelper.h"
#include "../Utils/S2W.h"
#include "../Utils/Base64.h"
#include "DownloadOneModel.h"


class ServerAPID {
	static inline  std::wstring Url(std::wstring part) {
		return ServerIPAPI + part;
	}
	static inline std::wstring Header(std::wstring part = L"") {
		return L"";
	}
private:

public:
	/*从服务器读取一个文件的下载地址
	返回值 success,errormsg,*/
	static std::wstring GetDownFileList(std::shared_ptr<DownloadOneModel> One) {
		if (nullptr == One || 0x123456789ABCDE0 != One->value) {
			One = nullptr;
			return L"指针错误";
		}
		
		return L"网络错误";
	}
};