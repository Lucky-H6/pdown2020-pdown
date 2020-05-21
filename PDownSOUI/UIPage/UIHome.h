#pragma once
#include <string>
#include "../Utils/singleton.h"
#include <souistd.h>
#include "../UIAdapter/HomeListAdapter.h"
#include "../DAL/DBHelper.h"
using namespace SOUI;

class UIHome
{
public:
	SINGLETON_DEFINE(UIHome);
	UIHome(void) {}
	~UIHome(void) {}
private:
	SWindow* _page = nullptr;
	SWindow* home_div_top = nullptr;
	SWindow* home_div_hot = nullptr;
	SEdit* home_input_search = nullptr;
	SButton* home_btn_search = nullptr;
	SWindow* home_list_view_head = nullptr;
	SListView* home_list_view = nullptr;
	SWindow* home_list_null = nullptr;
	SWindow* home_search = nullptr;
	HomeListAdapter* adapter = nullptr;
public:
	void InitPage(SOUI::SWindow* main) {
		_page = main;
		home_div_top = _page->FindChildByName("home_div_top");
		home_div_hot = _page->FindChildByName("home_div_hot");
		home_input_search = _page->FindChildByName2<SEdit>("home_input_search");
		home_input_search->GetEventSet()->subscribeEvent(EVT_KEYDOWN, Subscriber(&UIHome::EvtInputSearchKeyDown, this));
		home_btn_search = _page->FindChildByName2<SButton>("home_btn_search");
		home_btn_search->GetEventSet()->subscribeEvent(EVT_LBUTTONUP, Subscriber(&UIHome::EvtBtnSearchClick, this));
		home_list_view = _page->FindChildByName2<SListView>("home_list_view");
		home_list_view_head = _page->FindChildByName("home_list_view_head");
		home_list_null = _page->FindChildByName("home_list_null");
		home_search = _page->FindChildByName("home_search");
		adapter = new HomeListAdapter();
		adapter->list_null = home_list_null;
		home_list_view->SetAdapter(adapter);

		//_page->FindChildByName("downed_btn_deleteall")->GetEventSet()->subscribeEvent(EVT_LBUTTONUP, Subscriber(&UIHome::EvtBtnDeleteAllClick, this));

	}

	void SearchByUser(wstring userid) {
		userid = L"@user:" + userid;
		home_input_search->SetWindowTextW(userid.c_str());
		EvtBtnSearchClick(nullptr);
	}
	//显示搜索返回的结果列表
	void ShowSearchResult(vector<HomeItem> List) {
		home_btn_search->EnableWindow(true, true);
		home_list_view->SetVisible(true, true);
		home_list_view_head->SetVisible(true, true);
		home_search->SetVisible(false, true);
		adapter->AddHomeList(List);
	}

	bool EvtInputSearchKeyDown(EventArgs* pEvt)
	{
		EventKeyDown* evt = (EventKeyDown*)pEvt;
		if (evt->nChar == 13) {//回车
			EvtBtnSearchClick(NULL);
		}
		return true;
	}
	bool EvtBtnSearchClick(EventArgs* pEvt)
	{
		adapter->OnClearAll();
		wstring key = home_input_search->GetWindowTextW();
		if (key == L"") {
			home_div_top->SetVisible(true, true);
			home_div_hot->SetVisible(true, true);
			home_list_view->SetVisible(false, true);
			home_list_view_head->SetVisible(false, true);
			home_list_null->SetVisible(false, true);
			home_search->SetVisible(false, true);
			home_btn_search->EnableWindow(true, true);
		}
		else {
			home_div_top->SetVisible(false, true);
			home_div_hot->SetVisible(false, true);
			home_list_view->SetVisible(false, true);
			home_list_view_head->SetVisible(false, true);
			home_list_null->SetVisible(false, true);
			home_search->SetVisible(true, true);
			home_btn_search->EnableWindow(false, true);
			AppEvent::SendUI(UICmd::UIHome_SubmiteSearch, 0, key);
		}
		//IAnimation* pAni = SApplication::getSingletonPtr()->LoadAnimation(L"anim:home_height0");
		//home_div_top->SetAnimation(pAni);
		//pAni->Release();
		return true;
	}

	/*检测版本更新*/
	void CheckUP() {
		AppEvent::SendUI(UICmd::UIHome_CheckUP, 0, L"");
	}
};