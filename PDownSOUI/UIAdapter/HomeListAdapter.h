#pragma once
#include "../stdafx.h"
#include <vector>
#include <string>
#include <helper/SAdapterBase.h>
#include "HomeItem.h"
#include "../Utils/AppThread.h"
#include "../DAL/DBHelper.h"


class HomeListAdapter : public SAdapterBase
{
public:
	std::vector<HomeItem> DataSource = std::vector<HomeItem>();
	SWindow* list_null = nullptr;
	HomeListAdapter() {

	}
	~HomeListAdapter() {
		list_null = nullptr;
		DataSource.clear();
	}
	virtual int getCount() {
		return (int)DataSource.size();
	}
	/*当前选中项index*/
	int sel = -1;
	virtual void getView(int position, SWindow* pItem, pugi::xml_node xmlTemplate) {
		if (xmlTemplate.empty()) {
			return;
		}
		if (pItem->GetChildrenCount() == 0)
		{
			pItem->InitFromXml(xmlTemplate);
		}
		pItem->SetUserData(position);
		auto Item = &DataSource[position];
		try {
			pItem->GetEventSet()->subscribeEvent(EVT_CMD, Subscriber(&HomeListAdapter::EvtSelectItemChanged, this));
			auto div = pItem->FindChildByName(L"listitem");
			ULONG divflag = div->GetUserData();
			DWORD state = div->GetState();
			if (position == sel) div->ModifyState(WndState_Check, state, false);
			else {
				div->ModifyState(WndState_Normal, state, false);
				if (state == WndState_Check) pItem->Invalidate();
			}

			div->FindChildByName(L"listitem_filename")->SetWindowTextW(Item->FileName.c_str());
			div->FindChildByName(L"listitem_username")->SetWindowTextW(Item->UserName.c_str());

			div->FindChildByName2<SLink>(L"listitem_username")->GetEventSet()->subscribeEvent(EVT_LBUTTONUP, Subscriber(&HomeListAdapter::EvtLinkUserNameClick, this));
			SButton* home_btn_down = div->FindChildByName2<SButton>(L"home_btn_down");
			if (Item->Link != L"") {
				home_btn_down->SetVisible(true, true);
				home_btn_down->GetEventSet()->subscribeEvent(EVT_LBUTTONUP, Subscriber(&HomeListAdapter::EvtBtnDownClick, this));
			}
			else {
				home_btn_down->SetVisible(false, true);
			}
		}
		catch (std::exception& e) {
			SLOGFMTE(L"home getview error %hs", e.what());
		}
		catch (...) {
			SLOGFMTE(L"home getview error %ws", L"未定义错误");
		}
	}
private:
	void OnDataSetChanged() {
		if (list_null != nullptr) {
			bool isvis = list_null->IsVisible();//当前状态
			if (DataSource.size() == 0) {//应该显示
				if (isvis == false)list_null->SetVisible(true, true);//如果隐藏，刷新显示出来
			}
			else {//应该隐藏
				if (isvis == true)list_null->SetVisible(false, true);//如果显示，刷新隐藏
			}
		}
	}
	bool EvtSelectItemChanged(EventArgs* pEvt)
	{
		SWindow* pBtn = sobj_cast<SWindow>(pEvt->sender);
		sel = pBtn->GetUserData();
		notifyDataSetChanged();
		return true;
	}
	bool EvtLinkUserNameClick(EventArgs* pEvt)
	{
		SWindow* root = sobj_cast<SWindow>(pEvt->sender)->GetRoot();
		int position = root->GetUserData();
		auto Item = &DataSource[position];
		if (Item->UserID == L"") return true;
		AppEvent::SendUI(UICmd::UIHome_SearchByUser, 1, Item->UserID);
		return true;
	}
	bool EvtBtnDownClick(EventArgs* pEvt)
	{
		SWindow* root = sobj_cast<SWindow>(pEvt->sender)->GetRoot();
		int position = root->GetUserData();
		auto Item = &DataSource[position];
		if (Item->Link == L"") return true;
		AppEvent::SendUI(UICmd::UIMain_ShowAddNew, 1, L"@link:" + Item->Link);
		return true;
	}
public:

	void AddHomeList(std::vector<HomeItem>& List) {
		DataSource.swap(List);
		notifyDataSetChanged();
		OnDataSetChanged();
	}

	void OnClearAll() {
		DataSource.clear();
		notifyDataSetChanged();
		OnDataSetChanged();
	}
};