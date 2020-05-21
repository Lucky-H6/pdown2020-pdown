#pragma once
#include "../stdafx.h"
#include <vector>
#include <string>
#include <helper/SAdapterBase.h>
#include "DownItem.h"
#include "../Utils/OpenFileHelper.h"
#include "../Utils/AppThread.h"
#include "../DAL/DBHelper.h"


class DownedListAdapter : public SAdapterBase
{
public:
	std::vector<DownedItem> DataSource = std::vector<DownedItem>();
	SWindow* list_null = nullptr;
	DownedListAdapter() {

	}
	~DownedListAdapter() {
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
		auto Item = &DataSource[DataSource.size() - position - 1];
		try {
			pItem->GetEventSet()->subscribeEvent(EVT_CMD, Subscriber(&DownedListAdapter::EvtSelectItemChanged, this));
			pItem->GetEventSet()->subscribeEvent(EVT_ITEMPANEL_DBCLICK, Subscriber(&DownedListAdapter::EvtDBclickItem, this));
			auto div = pItem->FindChildByName(L"listitem");
			ULONG divflag = div->GetUserData();
			DWORD state = div->GetState();
			if (position == sel) div->ModifyState(WndState_Check, state, false);
			else {
				div->ModifyState(WndState_Normal, state, false);
				if (state == WndState_Check) pItem->Invalidate();
			}


			div->FindChildByName(L"listitem_filename")->SetWindowTextW(Item->server_filename.c_str());
			div->FindChildByName(L"listitem_filesize")->SetWindowTextW(Item->sizestr.c_str());
			div->FindChildByName(L"listitem_filetime")->SetWindowTextW(Item->down_timestr.c_str());

			div->FindChildByName2<SButton>(L"downed_btn_file")->GetEventSet()->subscribeEvent(EVT_LBUTTONUP, Subscriber(&DownedListAdapter::EvtBtnFileClick, this));
			div->FindChildByName2<SButton>(L"downed_btn_dir")->GetEventSet()->subscribeEvent(EVT_LBUTTONUP, Subscriber(&DownedListAdapter::EvtBtnDirClick, this));
			div->FindChildByName2<SButton>(L"downed_btn_delete")->GetEventSet()->subscribeEvent(EVT_LBUTTONUP, Subscriber(&DownedListAdapter::EvtBtnDeleteClick, this));
		}
		catch (std::exception & e) {
			SLOGFMTE(L"downed getview error %hs" ,e.what());
		}
		catch (...) {
			SLOGFMTE(L"downed getview error %ws", L"未定义错误");
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

public:

	void AddDownList(std::vector<DownedItem>& List) {
		DataSource.reserve(DataSource.size() + List.size() + 10);
		DataSource.insert(DataSource.end(), List.begin(), List.end());
		notifyDataSetChanged();
		OnDataSetChanged();
	}

	void AddDownItem(DownedItem& downed) {
		DataSource.push_back(downed);
		if (sel >= 0) sel++;
		notifyDataSetChanged();
		OnDataSetChanged();
	}


	void OnDeleteAll() {
		DataSource.clear();
		//删除DB
		AppThread::DBPool.enqueue([]() { DBHelper::GetI()->DelDownedAll(); });
		sel = -1;
		notifyDataSetChanged();
		OnDataSetChanged();
	}

	bool EvtBtnFileClick(EventArgs* pEvt)
	{
		SWindow* root = sobj_cast<SWindow>(pEvt->sender)->GetRoot();
		int position = root->GetUserData();
		auto Item = &DataSource[DataSource.size() - position - 1];
		//打开文件
		auto down_file = Item->down_file;
		if (down_file.length() == 0) return true;
		OpenFileHelper::OpenFile(down_file);
		return true;
	}

	bool EvtBtnDirClick(EventArgs* pEvt)
	{
		SWindow* root = sobj_cast<SWindow>(pEvt->sender)->GetRoot();
		int position = root->GetUserData();
		auto Item = &DataSource[DataSource.size() - position - 1];
		//打开目录
		auto down_file = Item->down_file;
		if (down_file.length() == 0) return true;
		OpenFileHelper::OpenDir(L"", down_file);
		return true;
	}
	bool EvtBtnDeleteClick(EventArgs* pEvt)
	{
		SWindow* root = sobj_cast<SWindow>(pEvt->sender)->GetRoot();
		int position = root->GetUserData();
		auto Item = &DataSource[DataSource.size() - position - 1];
		//删除DB
		AppThread::DBPool.enqueue([](wstring down_id) { DBHelper::GetI()->DelDowned(down_id); }, Item->down_id);
		DataSource.erase(DataSource.end() - position - 1);//删除记录

		notifyDataSetChanged();
		OnDataSetChanged();
		return true;
	}

	bool EvtDBclickItem(EventArgs* pEvt) {
		SWindow* root = sobj_cast<SWindow>(pEvt->sender)->GetRoot();
		int position = root->GetUserData();
		auto Item = &DataSource[DataSource.size() - position - 1];
		//打开文件
		auto down_file = Item->down_file;
		if (down_file.length() == 0) return true;
		OpenFileHelper::OpenFile(down_file);
		return true;
	}
};