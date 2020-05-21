#pragma once
#include <string>
#include "../Utils/singleton.h"
#include <souistd.h>
#include "../DAL/DBHelper.h"
using namespace SOUI;

class UIUser
{
public:
	SINGLETON_DEFINE(UIUser);
	UIUser(void) {}
	~UIUser(void) {}
private:
	SWindow* _page = nullptr;
	SEdit* user_input_phone = nullptr;
	SEdit* user_input_code = nullptr;
	SButton* user_btn_sendcode = nullptr;
	SButton* user_btn_login = nullptr;
	SWindow* login_bg1 = nullptr;
	SWindow* login_bg2 = nullptr;
	SWindow* login_bg3 = nullptr;
	SWindow* user_txt_errormsg = nullptr;
	SWindow* user_login = nullptr;
	SWindow* user_info = nullptr;
	SWindow* user_txt_phone = nullptr;
	int SendCodeTime = 0;
	int txtphone = 0;
public:
	void InitPage(SOUI::SWindow* main) {
		_page = main;
		login_bg1 = _page->FindChildByName("login_bg1");
		login_bg2 = _page->FindChildByName("login_bg2");
		login_bg3 = _page->FindChildByName("login_bg3");
		user_txt_errormsg = _page->FindChildByName("user_txt_errormsg");
		user_login = _page->FindChildByName("user_login");
		user_info = _page->FindChildByName("user_info");
		user_txt_phone = _page->FindChildByName("user_txt_phone");
		user_txt_phone->GetEventSet()->subscribeEvent(EVT_LBUTTONUP, Subscriber(&UIUser::EvtTxtPhone, this));

		user_input_phone = _page->FindChildByName2<SEdit>("user_input_phone");
		user_input_code = _page->FindChildByName2<SEdit>("user_input_code");
		user_btn_sendcode = _page->FindChildByName2<SButton>("user_btn_sendcode");
		user_btn_login = _page->FindChildByName2<SButton>("user_btn_login");
		user_btn_sendcode->GetEventSet()->subscribeEvent(EVT_LBUTTONUP, Subscriber(&UIUser::EvtBtnSendCode, this));
		user_btn_login->GetEventSet()->subscribeEvent(EVT_LBUTTONUP, Subscriber(&UIUser::EvtBtnLogin, this));
		user_input_phone->GetEventSet()->subscribeEvent(EVT_SETFOCUS, Subscriber(&UIUser::EvtEditPhoneFocus, this));
		user_input_phone->GetEventSet()->subscribeEvent(EVT_MOUSE_HOVER, Subscriber(&UIUser::EvtEditPhoneFocus, this));
		user_input_code->GetEventSet()->subscribeEvent(EVT_SETFOCUS, Subscriber(&UIUser::EvtEditCodeFocus, this));
		user_input_code->GetEventSet()->subscribeEvent(EVT_MOUSE_HOVER, Subscriber(&UIUser::EvtEditCodeFocus, this));

		wstring phone = DBHelper::GetI()->GetConfigData("UserPhone");
		wstring UserData = DBHelper::GetI()->GetUserData();
		if (UserData == L"") phone = L"";
		ShowLogin(phone);
	}
private:
	void ShowLogin(wstring phone) {
		bool islogin = (phone != L"");
		if (islogin) phone = L"已登录 " + phone.substr(0, 6) + L"***" + phone.substr(9);
		user_txt_phone->SetWindowTextW(phone.c_str());

		user_login->SetVisible(!islogin, true);
		user_info->SetVisible(islogin, true);
	}
	bool EvtTxtPhone(EventArgs* pEvt)
	{
		txtphone++;
		if (txtphone > 30) {//退出登录
			txtphone = 0;
			UILoginSuccess(L"", L"");
		}
		return true;
	}

	bool EvtEditPhoneFocus(EventArgs* pEvt)
	{
		login_bg1->SetVisible(false, true);
		login_bg2->SetVisible(true, true);
		login_bg3->SetVisible(false, true);
		return true;
	}

	bool EvtEditCodeFocus(EventArgs* pEvt)
	{
		login_bg1->SetVisible(false, true);
		login_bg2->SetVisible(false, true);
		login_bg3->SetVisible(true, true);
		return true;
	}

	bool EvtBtnSendCode(EventArgs* pEvt)
	{
		user_txt_errormsg->SetWindowTextW(L"");
		user_input_phone->KillFocus();
		user_input_code->SetWindowTextW(L"");
		wstring phone = user_input_phone->GetWindowTextW();
		if (phone.length() != 11 || phone[0] != '1') {
			user_txt_errormsg->SetWindowTextW(L"手机号格式错误");
			return true;
		}
		SendCodeTime = 30;
		user_btn_sendcode->EnableWindow(false, true);
		//api 验证
		AppEvent::SendUI(UICmd::UIUser_SendCode, 0, phone);
		return true;
	}
	bool EvtBtnLogin(EventArgs* pEvt)
	{
		login_bg1->SetVisible(true, true);
		login_bg2->SetVisible(false, true);
		login_bg3->SetVisible(false, true);

		user_txt_errormsg->SetWindowTextW(L"");
		user_input_phone->KillFocus();
		user_input_code->KillFocus();
		wstring phone = user_input_phone->GetWindowTextW();
		if (phone.length() != 11 || phone[0] != '1') {
			user_txt_errormsg->SetWindowTextW(L"手机号格式错误");
			return true;
		}

		wstring code = user_input_code->GetWindowTextW();
		if (code.length() != 6) {
			user_txt_errormsg->SetWindowTextW(L"验证码格式错误");
			return true;
		}
		user_btn_sendcode->EnableWindow(false, true);
		user_btn_login->EnableWindow(false, true);
		//api 验证 
		AppEvent::SendUI(UICmd::UIUser_Login, _wtoi(code.c_str()), phone);

		return true;
	}

public:
	void CheckIfSendCode() {
		if (SendCodeTime > 0) {
			wstring txt = L"等待(" + to_wstring(SendCodeTime) + L")";
			if (txt.length() < 6) txt += L" ";//为了让按钮保持长度
			user_btn_sendcode->SetWindowTextW(txt.c_str());
			SendCodeTime--;
			if (SendCodeTime < 0) SendCodeTime = 0;
			if (SendCodeTime == 0) {
				user_btn_sendcode->SetWindowTextW(L"发送短信");
				user_btn_sendcode->EnableWindow(true, true);
			}
		}
	}

	void UIErrorMsg(wstring msg) {
		user_txt_errormsg->SetWindowTextW(msg.c_str());
		if (SendCodeTime == 0) user_btn_sendcode->EnableWindow(true, true);
		user_btn_login->EnableWindow(true, true);
	}
	void UILoginSuccess(wstring phone, wstring data) {

		AppThread::DBPool.enqueue([phone, data]() {
			DBHelper::GetI()->SaveConfigData("UserPhone", phone);
			DBHelper::GetI()->SaveUserData(data);
			});

		ShowLogin(phone);
		user_txt_errormsg->SetWindowTextW(L"");
		if (SendCodeTime == 0) user_btn_sendcode->EnableWindow(true, true);
		user_btn_login->EnableWindow(true, true);
	}
};