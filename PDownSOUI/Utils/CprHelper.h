#pragma once
#include <functional>
#include <future>
#include <string>
#include <curl/curl.h>
#include <winhttp.h>
#include <sstream>
#pragma comment(lib, "Winhttp")
#include "AppThread.h"
#include <map>
using namespace std;

class CprHelper
{

public:
	struct Response {
		string url = "";
		string backheader = "";
		string backcookie = "";
		string backdata = "";
		string status_code = "";
		double elapsed = 0;
		bool IsStatus200() {
			return status_code == "200" || status_code == " 200" || status_code == "200 " || status_code == " 200 ";
		}
		bool IsStatus301() {
			return status_code == "301" || status_code == "302";
		}
		bool IsStatus403() {
			return status_code == "403" || status_code == "406";
		}
	};
	struct CurlHolder {
		CURL* handle = nullptr;
		struct curl_slist* headerchunk = nullptr;
		char error[CURL_ERROR_SIZE];
	};
protected:

	static std::vector<std::string> split(const std::string& to_split, char delimiter) {
		std::vector<std::string> tokens;
		std::stringstream stream(to_split);
		std::string item;
		while (std::getline(stream, item, delimiter)) {
			tokens.push_back(item);
		}
		return tokens;
	}

	// Get methods
public:
	static string U2A(const wstring& str)
	{
		string strDes;
		if (str.empty())
			goto __end;
		int nLen = ::WideCharToMultiByte(CP_ACP, 0, str.c_str(), str.size(), NULL, 0, NULL, NULL);
		if (0 == nLen)
			goto __end;
		char* pBuffer = new char[nLen + 1];
		memset(pBuffer, 0, nLen + 1);
		::WideCharToMultiByte(CP_ACP, 0, str.c_str(), str.size(), pBuffer, nLen, NULL, NULL);
		pBuffer[nLen] = '\0';
		strDes.append(pBuffer);
		delete[] pBuffer;
	__end:
		return strDes;
	}

	static CurlHolder* newHolder() {
		CurlHolder* holder = new CurlHolder();
		holder->handle = curl_easy_init();
		holder->headerchunk = nullptr;
		return holder;
	}
	static void freeHolder(CurlHolder* holder) {
		curl_easy_cleanup(holder->handle);
		if (holder->headerchunk) curl_slist_free_all(holder->headerchunk);
		delete holder;
	}

	static void initOption(CurlHolder* holder, string method, wstring url, bool FOLLOWLOCATION) {
		auto curl = holder->handle;
		if (!curl)  return;
		try {
			if (DNSShared) curl_easy_setopt(curl, CURLOPT_SHARE, DNSShared);    ///创建easy_handle并设置器share属性
			curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, (FOLLOWLOCATION ? 1L : 0));
			curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
			curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 50L);
			holder->error[0] = '\0';
			curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, holder->error);
			curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "");
			curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);

			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0); // 对认证证书来源的检查
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0); // 从证书中检查SSL加密算法是否存在


			curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 15);//连接超时
			curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, 60);//字节/秒
			curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, 25);//秒

			if (method == "GET") {
				curl_easy_setopt(curl, CURLOPT_NOBODY, 0L);
				curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");
			}
			else if (method == "POST") {
				curl_easy_setopt(curl, CURLOPT_NOBODY, 0L);
				curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
			}
			else if (method == "HEAD") {
				curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
				curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "HEAD");
			}
			string urls = U2A(url);
			curl_easy_setopt(curl, CURLOPT_URL, urls.data());
		}
		catch (...) {}

	}

	static void initProxy(CurlHolder* holder) {
		if (!holder->handle)  return;
		try {
			WINHTTP_CURRENT_USER_IE_PROXY_CONFIG ieProxyConfig;
			if (::WinHttpGetIEProxyConfigForCurrentUser(&ieProxyConfig) && ieProxyConfig.lpszProxy != NULL)
			{

				wstring ip = ieProxyConfig.lpszProxy;
				string ips = U2A(ip);
				auto tokens = split(ips, ';');
				for (auto item = tokens.begin(); item != tokens.end(); item++)
				{
					if (item->length() > 0)
					{
						string p = *item + "";

						if (p.find("http=")) {
							p = "http://" + p.substr(p.find("=") + 1);
						}
						else if (p.find("https=")) {
							p = "http://" + p.substr(p.find("=") + 1);
						}
						else if (p.find("socket=")) {
							p = "http://" + p.substr(p.find("=") + 1);
						}
						else continue;
						curl_easy_setopt(holder->handle, CURLOPT_PROXY, p.data());
					}
				}
			}
			if (ieProxyConfig.lpszAutoConfigUrl != NULL)
			{
				::GlobalFree(ieProxyConfig.lpszAutoConfigUrl);
			}
			if (ieProxyConfig.lpszProxy != NULL)
			{
				::GlobalFree(ieProxyConfig.lpszProxy);
			}
			if (ieProxyConfig.lpszProxyBypass != NULL)
			{
				::GlobalFree(ieProxyConfig.lpszProxyBypass);
			}
		}
		catch (...) {}
	}
	static void initHeader(CurlHolder* holder, wstring& header) {
		if (header.length() < 4) return;
		auto curl = holder->handle;
		if (curl) {
			try {
				string headers = U2A(header);
				curl_slist* list = nullptr;
				auto tokens = split(headers, '\n');
				for (auto item = tokens.begin(); item != tokens.end(); item++)
				{
					if (item->length() > 0)
					{
						string p = *item + "";
						if (p[p.length() - 1] == '\r') p = p.substr(0, p.length() - 1);
						if (p.find(':') != string::npos)	list = curl_slist_append(list, p.data());
					}
				}
				curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
				holder->headerchunk = list;
			}
			catch (...) {}
		}
	}
	static string GetRespCookie(CurlHolder* holder) {
		std::string cookies = "";;
		auto curl = holder->handle;
		if (curl) {
			try {
				struct curl_slist* raw_cookies;
				curl_easy_getinfo(curl, CURLINFO_COOKIELIST, &raw_cookies);
				for (struct curl_slist* nc = raw_cookies; nc; nc = nc->next) {
					auto tokens = split(nc->data, '\t');
					auto value = tokens.back();
					tokens.pop_back();
					auto key = tokens.back();
					cookies += key + "=" + value + "; ";
				}
				if (cookies.length() > 0) cookies = cookies.substr(0, cookies.length() - 2);
				curl_slist_free_all(raw_cookies);
			}
			catch (...) {}
		}
		return cookies;
	}
	static string GetRespHeader(string& header_string) {
		string fixhead = "";
		try {
			string http = "";
			string setcookie = "";
			bool IsProxy = false;
			map<string, string> hmap;
			auto tokens = split(header_string, '\n');
			for (auto item = tokens.begin(); item != tokens.end(); item++)
			{
				if (item->length() > 2)
				{
					string p = *item + "";
					if (p[p.length() - 1] == '\r') p = p.substr(0, p.length() - 1);
					auto find = p.find(':');
					if (find != string::npos) {
						string key = p.substr(0, find);
						string value = p.substr(find + 1);
						if (key == "Set-Cookie") setcookie += p + "\n";
						else hmap[key] = value;
					}
					else if (p.find("HTTP/") != string::npos) {
						http = p;
						if (p.find("HTTP/1.1 200 Connection Established") != string::npos) IsProxy = true;
					}
				}
			}
			tokens.clear();
			//hmap["Content-Length"] =to_string( (long)content);
			fixhead = http + "\n";
			for (auto item = hmap.begin(); item != hmap.end(); item++)
			{
				fixhead += item->first + ": " + item->second + "\n";
			}
			hmap.clear();
			fixhead += setcookie;
			fixhead += "proxy: " + to_string(IsProxy) + "\n";
		}
		catch (...) {}

		return fixhead;

	}
private:
	static size_t writeFunction(void* ptr, size_t size, size_t nmemb, std::string* data) {
		data->append(static_cast<char*>(ptr), size * nmemb);
		return size * nmemb;
	}

	static void SendRequest(CurlHolder* holder, Response& resp) {
		auto curl = holder->handle;
		if (curl) {
			try {
				std::string response_string;
				std::string header_string;
				curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CprHelper::writeFunction);
				curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);
				curl_easy_setopt(curl, CURLOPT_HEADERDATA, &header_string);
				auto curl_error = curl_easy_perform(curl);
				char* raw_url;
				long response_code;
				double elapsed;
				double content;
				long redirect;
				curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
				curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &elapsed);
				curl_easy_getinfo(curl, CURLINFO_EFFECTIVE_URL, &raw_url);
				curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &content);
				curl_easy_getinfo(curl, CURLINFO_REDIRECT_COUNT, &redirect);

				std::string cookies = GetRespCookie(holder);
				string fixhead = GetRespHeader(header_string);

				fixhead += "Cookie: " + cookies + "\n";
				fixhead += "raw_url: " + string(raw_url) + "\n";
				fixhead += "status_code: " + to_string(response_code) + "\n";
				fixhead += "elapsed: " + to_string(elapsed) + "\n";
				fixhead += "redirect: " + to_string(redirect);

				resp.backheader = fixhead;
				resp.backdata = response_string;
				resp.backcookie = cookies;
				resp.status_code = to_string(response_code);
				resp.url = string(raw_url);
				resp.elapsed = elapsed;

				if (curl_error != CURLE_OK) {
					resp.status_code = "10005";
					resp.backheader = "HTTP/1.1 10005 NetError" + to_string(curl_error) + string(holder->error);
				}
			}
			catch (...) {
				resp.status_code = "10002";
				resp.backheader = "HTTP/1.1 10002 NetError";
			}
		}
		else {
			resp.status_code = "10001";
			resp.backheader = "HTTP/1.1 10001 NetError";
		}
	}

public:
	static CURLSH* DNSShared;
	static void GobalInit() {
		curl_global_init(CURL_GLOBAL_ALL);
		DNSShared = curl_share_init();   ///创建一个share_handle
		curl_share_setopt(DNSShared, CURLSHOPT_SHARE, CURL_LOCK_DATA_DNS);   ///设置在盖share_handle上共享的cookie
	}
	static void GobalClean() {
		if (DNSShared) curl_share_cleanup(DNSShared);
		curl_global_cleanup();
	}
	static Response Get(wstring url, wstring header, bool FOLLOWLOCATION) {
		Response resp = Response();
		try {
			CurlHolder* holder = newHolder();
			// Set up some sensible defaults
			initOption(holder, "GET", url, FOLLOWLOCATION);
			initHeader(holder, header);
			initProxy(holder);
			SendRequest(holder, resp);
			freeHolder(holder);
		}
		catch (...) {
			resp.status_code = "10003";
			resp.backheader = "HTTP/1.1 10003 NetError";
		}
		return resp;
	}

	//尝试3次，只要成功200立即返回，反之返回第3次的错误结果
	static Response Get3(wstring url, wstring header, bool FOLLOWLOCATION) {
		Response resp = Response();
		for (int i = 0; i < 3; i++) {
			resp = Get(url, header, FOLLOWLOCATION);
			if (resp.IsStatus200() || resp.IsStatus301() || resp.IsStatus403()) return resp;
		}
		return resp;
	}

	// Get async methods
	static auto Get3Async(wstring& url, wstring& header, std::function<void(Response)> const& callback) {
		AppThread::HttpPool.enqueue([](wstring url, wstring header, std::function<void(Response)> const& callback) {callback(Get3(url, header, true));	},
			url, header, callback);
	}
	static Response Post(wstring url, wstring header, string& postdata, bool FOLLOWLOCATION) {
		Response resp = Response();
		try {
			CurlHolder* holder = newHolder();
			// Set up some sensible defaults
			initOption(holder, "POST", url, FOLLOWLOCATION);
			initHeader(holder, header);
			initProxy(holder);
			curl_easy_setopt(holder->handle, CURLOPT_POSTFIELDSIZE, postdata.length());
			curl_easy_setopt(holder->handle, CURLOPT_POSTFIELDS, postdata.data());
			SendRequest(holder, resp);
			freeHolder(holder);
		}
		catch (...) {
			resp.status_code = "10004";
			resp.backheader = "HTTP/1.1 10004 NetError";
		}
		return resp;
	}

	static Response Post3(wstring url, wstring header, string& postdata, bool FOLLOWLOCATION) {
		Response resp = Response();
		for (int i = 0; i < 3; i++) {
			resp = Post(url, header, postdata, FOLLOWLOCATION);
			if (resp.IsStatus200() || resp.IsStatus301() || resp.IsStatus403()) return resp;
		}
		return resp;
	}

	// Get async methods
	static auto Post3Async(wstring& url, wstring& header, string& postdata, std::function<void(Response)> const& callback) {
		AppThread::HttpPool.enqueue([](wstring url, wstring header, string& postdata, std::function<void(Response)> const& callback) {callback(Post3(url, header, postdata, true));	},
			url, header, postdata, callback);
	}

};