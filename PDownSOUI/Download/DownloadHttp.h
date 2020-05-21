#pragma once
#include "../Utils/CprHelper.h"
#include "DownloadHttpModel.h"

#ifndef CheckNull
#define CheckNull(One) \
if (nullptr == One || 0x123456789ABCDE0 != One->value) {\
	One = nullptr;\
}
#define CheckNull_R(One) \
if (nullptr == One || 0x123456789ABCDE0 != One->value) {\
	One = nullptr;\
	return;\
}
#define CheckNull_RF(One) \
if (nullptr == One || 0x123456789ABCDE0 != One->value) {\
	One = nullptr;\
	return false;\
}
#endif

class DownloadHttp {
	static inline std::wstring Header(std::wstring part = L"") {
		return L"";
	}
public:
	/*下载指定的文件 自动ReportDowning
	下载成功返回true (resp->status_code=206 && resp->errormsg == "")
	失败返回false 错误信息保存在resp->errormsg
	*/
	static bool DownFile(wstring method, wstring url, wstring header, DownloadResp*& resp) {
		//SLOGFMTI("DownFile %ws", url.c_str());
		CheckNull_RF(resp);
		try {
			CprHelper::CurlHolder* holder = CprHelper::newHolder();
			CprHelper::initOption(holder, (method == L"get" ? "GET" : "POST"), url, true);
			//下载文件特殊处理一下超时问题
			curl_easy_setopt(holder->handle, CURLOPT_LOW_SPEED_LIMIT, 60);//字节/秒
			curl_easy_setopt(holder->handle, CURLOPT_LOW_SPEED_TIME, 60);//
			header = Header(header);
			CprHelper::initHeader(holder, header);
			CprHelper::initProxy(holder);
			SendDownRequest(holder, resp);
			CprHelper::freeHolder(holder);
			CheckNull_RF(resp);
			resp->FlushToFile();
			//SLOGFMTI("DownFile return %Ts", resp->status_code.c_str());
			if (resp->errormsg == L"" && (resp->status_code == "200" || resp->status_code == "206")) return true;
			else {
				return false;
			}
		}
		catch (...) {
			CheckNull_RF(resp);
			resp->status_code = "10003";
			resp->errormsg = L"DownFile 网络错误";
			SLOGFMTE("DownFile catch %ws", resp->errormsg.c_str());
			return false;
		}

	}
private:
	static void SendDownRequest(CprHelper::CurlHolder* holder, DownloadResp*& resp) {
		//SLOGFMTI("SendDownRequest %ws", resp->errormsg.c_str());
		auto curl = holder->handle;
		if (curl) {
			try {
				//std::string header_string;

				curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
				curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, progressFunc);  //设置回调的进度函数
				curl_easy_setopt(curl, CURLOPT_XFERINFODATA, resp);   //此设置对应上面的const char *flag

				curl_easy_setopt(curl, CURLOPT_HEADER, 0);
				curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunction);
				curl_easy_setopt(curl, CURLOPT_WRITEDATA, resp);


				string range = resp->GetRequestRange();
				if (range.length() > 0)	curl_easy_setopt(curl, CURLOPT_RANGE, range.data());

				auto curl_error = curl_easy_perform(curl);

				long response_code;
				curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

				CheckNull_R(resp);
				resp->status_code = to_string(response_code);
				if (curl_error != CURLE_OK) {
					resp->status_code = "1000" + to_string((int)curl_error);
					resp->errormsg = L"curl_error 网络错误";
				}
				//SLOGFMTI("SendDownRequest return %ws", resp->errormsg.c_str());
			}
			catch (...) {
				CheckNull_R(resp);
				resp->status_code = "10002";
				resp->errormsg = L"response 网络错误";
				SLOGFMTE("SendDownRequest catch %ws", resp->errormsg.c_str());
			}
		}
		else {
			CheckNull_R(resp);
			resp->status_code = "10001";
			resp->errormsg = L"curl 网络错误";
			SLOGFMTE("SendDownRequest catch %ws", resp->errormsg.c_str());
		}
	}
	static size_t writeFunction(char* ptr, size_t size, size_t nmemb, void* userdata) {
		DownloadResp* resp = (DownloadResp*)userdata;
		if (nullptr == resp || 0x123456789ABCDE0 != resp->value) {
		}
		else resp->WriteToFile(ptr, size, nmemb);
		return size * nmemb;
	}
	/*只是为了终止下载*/
	static int progressFunc(void* userdata, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) {
		DownloadResp* resp = (DownloadResp*)userdata;
		if (nullptr == resp || 0x123456789ABCDE0 != resp->value) {
			return 1;//被销毁了
		}
		else {
			if (resp->StopDownload) return 1;//返回非0值，取消下载
			else if (resp->errormsg != L"") return 1;//返回非0值，取消下载
			else {
				resp->ReportDowning();
				return 0;
			}
		}
	}

};