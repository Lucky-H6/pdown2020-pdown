#pragma once
#include <string>

#include "../Utils/singleton.h"
using namespace SOUI;

class UIMain
{
public:
	SINGLETON_DEFINE(UIMain);
	UIMain(void) {}
	~UIMain(void) {}
private:
	SWindow* _page = nullptr;
	STabCtrl* menutabctrl = nullptr;
	SRadioBox* menuitem_home = nullptr;
	SRadioBox* menuitem_downing = nullptr;
	SRadioBox* menuitem_downed = nullptr;
	SRadioBox* menuitem_user = nullptr;
	SRadioBox* menuitem_setting = nullptr;
	int selecttabid = -1;
	bool isaddnewpagebydowning = false;
	bool isaddnewpagebyhome = false;
public:
	void InitPage(SOUI::SWindow* main) {
		isaddnewpagebydowning = false;
		isaddnewpagebyhome = false;
		_page = main;
		menutabctrl = main->FindChildByName2<STabCtrl>("menutabctrl");
		menuitem_home = main->FindChildByName2<SRadioBox>("menuitem_home");
		menuitem_home->GetEventSet()->subscribeEvent(EVT_STATECHANGED, Subscriber(&UIMain::EvtMenuItemChecked, this));
		menuitem_home->GetEventSet()->subscribeEvent(EVT_LBUTTONUP, Subscriber(&UIMain::EvtMenuItemClick, this));
		menuitem_downing = main->FindChildByName2<SRadioBox>("menuitem_downing");
		menuitem_downing->GetEventSet()->subscribeEvent(EVT_STATECHANGED, Subscriber(&UIMain::EvtMenuItemChecked, this));
		menuitem_downing->GetEventSet()->subscribeEvent(EVT_LBUTTONUP, Subscriber(&UIMain::EvtMenuItemClick, this));
		menuitem_downed = main->FindChildByName2<SRadioBox>("menuitem_downed");
		menuitem_downed->GetEventSet()->subscribeEvent(EVT_STATECHANGED, Subscriber(&UIMain::EvtMenuItemChecked, this));
		menuitem_downed->GetEventSet()->subscribeEvent(EVT_LBUTTONUP, Subscriber(&UIMain::EvtMenuItemClick, this));
		menuitem_user = main->FindChildByName2<SRadioBox>("menuitem_user");
		menuitem_user->GetEventSet()->subscribeEvent(EVT_STATECHANGED, Subscriber(&UIMain::EvtMenuItemChecked, this));
		menuitem_user->GetEventSet()->subscribeEvent(EVT_LBUTTONUP, Subscriber(&UIMain::EvtMenuItemClick, this));
		menuitem_setting = main->FindChildByName2<SRadioBox>("menuitem_setting");
		menuitem_setting->GetEventSet()->subscribeEvent(EVT_STATECHANGED, Subscriber(&UIMain::EvtMenuItemChecked, this));
		menuitem_setting->GetEventSet()->subscribeEvent(EVT_LBUTTONUP, Subscriber(&UIMain::EvtMenuItemClick, this));

		
	}
	void ShowAddNew(bool isShow, wstring Link) {
		std::wstring tabname = L"";

		if (isShow) {
			if (Link != L"") {
				tabname = L"menuitem_home";
				isaddnewpagebyhome = true;
				isaddnewpagebydowning = false;
			}
			else {
				tabname = L"menuitem_downing";
				isaddnewpagebyhome = false;
				isaddnewpagebydowning = true;
			}
		}
		else {
			if (isaddnewpagebyhome) tabname = L"menuitem_home";
			else tabname = L"menuitem_downing";
			isaddnewpagebyhome = false;
			isaddnewpagebydowning = false;
		}
		RealSelectPage(tabname);
	}

	void RefreshUI() {
		_page->Invalidate();
	}
	void OnTestUI() {
		auto ui = menutabctrl->SetCurSel(0);
	}
private:
	/*真正的跳转，PageMain内部调用*/
	void RealSelectPage(std::wstring& tabname) {
		int tabid = 0;
		if (tabname == L"menuitem_home") {
			if (isaddnewpagebyhome)	tabid = 1;
			else tabid = 0;
		}
		else if (tabname == L"menuitem_downing") {
			if (isaddnewpagebydowning)	tabid = 1;
			else tabid = 2;
		}
		else if (tabname == L"menuitem_downed") tabid = 3;
		else if (tabname == L"menuitem_user") tabid = 4;
		else if (tabname == L"menuitem_setting") tabid = 5;

		if (selecttabid != tabid) {
			selecttabid = tabid;
			menutabctrl->SetCurSel(tabid);
			//已经跳转到某一页触发对应页面的数据刷新
			//AppMsg::SendPAGE(tabname, tabid);
		}
	}
	/*点击导航菜单(tab_setting->SetCheck / 用户点击)*/
	bool EvtMenuItemChecked(EventArgs* pEvt) {
		auto evt = (EventSwndStateChanged*)pEvt;
		if (evt->dwNewState == WndState_Check) {
			//WndState_Check
			std::wstring tabname = std::wstring(pEvt->nameFrom);
			RealSelectPage(tabname);
		}
		return true;
	}
	/*点击导航菜单(用户点击)*/
	bool EvtMenuItemClick(EventArgs* pEvt) {
		std::wstring tabname = std::wstring(pEvt->nameFrom);
		RealSelectPage(tabname);
		return true;
	}
};