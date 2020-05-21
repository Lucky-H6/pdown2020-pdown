#pragma once
#include <string>
#include "../Utils/singleton.h"
#include <souistd.h>
#include "../UIAdapter/DowningListAdapter.h"
#include "../UIAdapter/DownItem.h"
#include "../DAL/DBHelper.h"
#include "UIAddNew.h"	
#include "../AppInfo.h"	
using namespace SOUI;

class UIDowning
{
public:
	SINGLETON_DEFINE(UIDowning);
	UIDowning(void) {}
	~UIDowning(void) {}
private:
	SWindow* _page = nullptr;
	SListView* downing_list_view = nullptr;
	SWindow* downing_list_null = nullptr;
	DowningListAdapter* adapter = nullptr;
public:
	void InitPage(SOUI::SWindow* main) {
		_page = main;
		downing_list_view = _page->FindChildByName2<SListView>("downing_list_view");
		downing_list_null = _page->FindChildByName("downing_list_null");
		adapter = new DowningListAdapter();
		adapter->list_null = downing_list_null;
		downing_list_view->SetAdapter(adapter);

		_page->FindChildByName("downing_btn_addnew")->GetEventSet()->subscribeEvent(EVT_LBUTTONUP, Subscriber(&UIDowning::EvtBtnAddNewClick, this));
		_page->FindChildByName("downing_btn_startall")->GetEventSet()->subscribeEvent(EVT_LBUTTONUP, Subscriber(&UIDowning::EvtBtnStartAllClick, this));
		_page->FindChildByName("downing_btn_stopall")->GetEventSet()->subscribeEvent(EVT_LBUTTONUP, Subscriber(&UIDowning::EvtBtnStopAllClick, this));
		_page->FindChildByName("downing_btn_deleteall")->GetEventSet()->subscribeEvent(EVT_LBUTTONUP, Subscriber(&UIDowning::EvtBtnDeleteAllClick, this));



		std::vector<DowningItem> List;
		DBHelper::GetI()->GetDowningList(List);
		adapter->AddNewList(List);
		List.clear();
	}

	void AddNew(std::vector<DowningItem>& List) {
		adapter->AddNewList(List);
		if (List.size() > 0) {
			AppThread::DBPool.enqueue([](std::vector<DowningItem> List) { DBHelper::GetI()->AddDowningList(List); }, List);
		}
	}

	void UpdateDownItemState(std::wstring cmd) {
		auto lines = StringHelper::Split(cmd, L',');
		wstring down_id = lines[0];
		wstring down_state = lines[1];
		wstring down_speed = lines[2];
		size_t down_prog = (size_t)_wtol(lines[3].c_str());
		int64_t down_size = _wtoi64(lines[4].c_str());
		wstring down_timeleft = lines[5];
		adapter->UpdateDownItemState(down_id, down_state, down_speed, down_prog, down_size, down_timeleft);
	}
	void DeleteDownItem(std::wstring down_id) {
		adapter->DeleteDownItem(down_id);
	}

	void FireDownloader() {
		AppInfo::GetI()->IsDowning= adapter->FireDownloader();
	}

private:
	/*点击新建下载*/
	bool EvtBtnAddNewClick(EventArgs* pEvt) {
		AppEvent::SendUI(UICmd::UIMain_ShowAddNew, 1, L"");
		return true;
	}
	/*点击全部开始*/
	bool EvtBtnStartAllClick(EventArgs* pEvt) {
		adapter->OnStartAll();
		return true;
	}
	/*点击全部暂停*/
	bool EvtBtnStopAllClick(EventArgs* pEvt) {
		adapter->OnStopAll();
		return true;
	}
	/*点击全部删除*/
	bool EvtBtnDeleteAllClick(EventArgs* pEvt) {
		adapter->OnDeleteAll();
		return true;
	}
};