#pragma once
#include <string>
#include "../jsoncpp/include/json/json.h"
#include <souistd.h>
#include "S2W.h"
class JsonHelper {
public:

	/*检查返回的json 确保 正确转换*/
	static std::wstring GetJson(std::string jsonstr, Json::Value& json_object) {
		try {
			bool res;
			JSONCPP_STRING errs;
			Json::CharReaderBuilder readerBuilder;
			std::unique_ptr<Json::CharReader> const jsonReader(readerBuilder.newCharReader());
			res = jsonReader->parse(jsonstr.c_str(), jsonstr.c_str() + jsonstr.length(), &json_object, &errs);
			if (!res || !errs.empty()) {
				SLOGFMTE(L"parseJsonError outjson=%hs", jsonstr.c_str());
				return L"error";
			}

			return L"success";
		}
		catch (...) {
			SLOGFMTE(L"unkownError %d", 2);
			return L"error";
		}
	}

	/*检查返回的json 确保 正确转换+读取errno*/
	static std::wstring GetJsonWith_errno(std::string jsonstr, Json::Value& json_object, int& iserrno) {
		try {
			bool res;
			JSONCPP_STRING errs;
			Json::CharReaderBuilder readerBuilder;
			std::unique_ptr<Json::CharReader> const jsonReader(readerBuilder.newCharReader());
			res = jsonReader->parse(jsonstr.c_str(), jsonstr.c_str() + jsonstr.length(), &json_object, &errs);
			if (!res || !errs.empty()) {
				SLOGFMTE(L"parseJsonError outjson=%hs", jsonstr.c_str());
				return L"error";
			}

			iserrno = json_object["errno"].asInt();
			if (iserrno != 0) {
				SLOGFMTE(L"iserrno outjson=%hs", jsonstr.c_str());
				return L"error";
			}
			return L"success";
		}
		catch (...) {
			SLOGFMTE(L"unkownError %d", 2);
			return L"error";
		}
	}

	/*检查返回的json 确保 正确转换+state==success*/
	static std::wstring GetJsonWith_state(std::string jsonstr, Json::Value& json_object) {
		try {
			bool res;
			JSONCPP_STRING errs;
			Json::CharReaderBuilder readerBuilder;
			std::unique_ptr<Json::CharReader> const jsonReader(readerBuilder.newCharReader());
			res = jsonReader->parse(jsonstr.c_str(), jsonstr.c_str() + jsonstr.length(), &json_object, &errs);
			if (!res || !errs.empty()) {
				SLOGFMTE(L"parseJsonError outjson=%hs", jsonstr.c_str());
				return L"error";
			}

			std::string s = json_object["state"].asString();
			std::string m = json_object["message"].asString();
			if (s != "success") return S2W::StringtoWString(m, CP_UTF8);//返回的错误信息

			return L"success";
		}
		catch (...) {
			SLOGFMTE(L"unkownError %d", 2);
			return L"error";
		}
	}
};