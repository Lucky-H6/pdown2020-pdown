#pragma once
#include <string>
#include "../jsoncpp/include/json/json.h"
#include "../Utils/CprHelper.h"
#include "../Utils/S2W.h"
#include "ServerAPI_User.h"
#include "../UIPage/UIUser.h"
#include "event/SNotifyCenter.h"
class DALUser {
public:
	static void SendCode(std::wstring phone) {
		wstring ret = ServerAPI_User::SendCode(phone);
		if (ret == L"success") {//出错
			ret = L"短信已发送,请注意查收";
		}

		SNotifyCenter::getSingletonPtr()->RunOnUIAsync([ret] {
			UIUser::GetI()->UIErrorMsg(ret);
			});
	}
	static void Login(std::wstring phone, std::wstring code) {
		wstring data = L"";
		wstring ret = ServerAPI_User::Login(phone, code, data);
		if (ret == L"success") {
			SNotifyCenter::getSingletonPtr()->RunOnUIAsync([phone, data] {
				UIUser::GetI()->UILoginSuccess(phone, data);
				});
		}
		else {
			SNotifyCenter::getSingletonPtr()->RunOnUIAsync([ret] {
				UIUser::GetI()->UIErrorMsg(ret);
				});
		}
	}
};