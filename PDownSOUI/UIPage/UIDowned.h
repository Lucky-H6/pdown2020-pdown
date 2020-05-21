#pragma once
#include <string>
#include "../Utils/singleton.h"
#include <souistd.h>
#include "../UIAdapter/DownedListAdapter.h"
#include "../UIAdapter/DownItem.h"
#include "../DAL/DBHelper.h"
using namespace SOUI;

class UIDowned
{
public:
	SINGLETON_DEFINE(UIDowned);
	UIDowned(void) {}
	~UIDowned(void) {}
private:
	SWindow* _page = nullptr;
	SListView* downed_list_view = nullptr;
	SWindow* downed_list_null = nullptr;
	DownedListAdapter* adapter = nullptr;
public:
	void InitPage(SOUI::SWindow* main) {
		_page = main;
		downed_list_view = _page->FindChildByName2<SListView>("downed_list_view");
		downed_list_null = _page->FindChildByName("downed_list_null");
		adapter = new DownedListAdapter();
		adapter->list_null = downed_list_null;
		downed_list_view->SetAdapter(adapter);
		_page->FindChildByName("downed_btn_deleteall")->GetEventSet()->subscribeEvent(EVT_LBUTTONUP, Subscriber(&UIDowned::EvtBtnDeleteAllClick, this));

		std::vector<DownedItem> List;
		DBHelper::GetI()->GetDownedList(List);
		adapter->AddDownList(List);
		List.clear();
	}

	void AddDownItem(std::wstring down_id) {
		DownedItem model = DownedItem();
		bool IsGet=DBHelper::GetI()->GetDowned(down_id, model);
		if(IsGet) adapter->AddDownItem(model);
	}

	/*µã»÷È«²¿É¾³ý*/
	bool EvtBtnDeleteAllClick(EventArgs* pEvt) {
		adapter->OnDeleteAll();
		return true;
	}
	
};