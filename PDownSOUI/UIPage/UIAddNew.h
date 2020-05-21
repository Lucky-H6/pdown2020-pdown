#pragma once
#include <string>
#include "../Utils/singleton.h"
#include "../Utils/CodeMD5.h"
#include <souistd.h>
#include "UISetting.h"	
#include "UIDowning.h"	
#include "../trayicon/STreeFile.h"
using namespace SOUI;
using namespace std;


class UIAddNew
{
public:
	SINGLETON_DEFINE(UIAddNew);
	UIAddNew(void) {}
	~UIAddNew(void) {}
private:
	SWindow* _page = nullptr;
	SButton* addnew_btn_submite = nullptr;
	SButton* addnew_btn_refresh = nullptr;
	SButton* addnew_btn_chance = nullptr;
	SWindow* addnew_div_linktip = nullptr;
	SWindow* addnew_div_step = nullptr;
	SWindow* addnew_div_tree = nullptr;

	SWindow* addnew_filetree_state = nullptr;
	SEdit* addnew_input_link = nullptr;
	STreeFile* addnew_tree_view = nullptr;

	SButton* addnew_btn_selectdir = nullptr;
	SEdit* addnew_input_downdir = nullptr;
	SButton* addnew_btn_downselect = nullptr;
	SWindow* addnew_input_showmsg = nullptr;
	SWindow* addnew_step_error = nullptr;


	wstring Link = L"";
public:
	void InitPage(SOUI::SWindow* main) {
		_page = main;

		addnew_input_link = _page->FindChildByName2<SEdit>("addnew_input_link");
		addnew_input_link->GetEventSet()->subscribeEvent(EVT_KEYDOWN, Subscriber(&UIAddNew::EvtInputLinkKeyDown, this));
		addnew_btn_submite = _page->FindChildByName2<SButton>("addnew_btn_submite");
		addnew_btn_submite->GetEventSet()->subscribeEvent(EVT_LBUTTONUP, Subscriber(&UIAddNew::EvtBtnSubmiteClick, this));

		addnew_btn_refresh = _page->FindChildByName2<SButton>("addnew_btn_refresh");
		addnew_btn_refresh->GetEventSet()->subscribeEvent(EVT_LBUTTONUP, Subscriber(&UIAddNew::EvtBtnRefreshClick, this));


		addnew_btn_chance = _page->FindChildByName2<SButton>("addnew_btn_chance");
		addnew_btn_chance->GetEventSet()->subscribeEvent(EVT_LBUTTONUP, Subscriber(&UIAddNew::EvtBtnChanceClick, this));
		auto addnew_step_anim = _page->FindChildByName("addnew_step_anim");
		IAnimation* pAni = SApplication::getSingletonPtr()->LoadAnimation(L"anim:addnew_step");
		addnew_step_anim->SetAnimation(pAni);
		pAni->Release();

		addnew_div_linktip = _page->FindChildByName("addnew_div_linktip");
		addnew_div_step = _page->FindChildByName("addnew_div_step");
		addnew_div_tree = _page->FindChildByName("addnew_div_tree");

		addnew_btn_selectdir = _page->FindChildByName2<SButton>("addnew_btn_selectdir");
		addnew_btn_selectdir->GetEventSet()->subscribeEvent(EVT_LBUTTONUP, Subscriber(&UIAddNew::EvtBtnSelectDirClick, this));
		addnew_input_downdir = _page->FindChildByName2<SEdit>("addnew_input_downdir");
		addnew_btn_downselect = _page->FindChildByName2<SButton>("addnew_btn_downselect");
		addnew_btn_downselect->GetEventSet()->subscribeEvent(EVT_LBUTTONUP, Subscriber(&UIAddNew::EvtBtnDownSelectClick, this));

		addnew_filetree_state = _page->FindChildByName("addnew_filetree_state");
		addnew_tree_view = _page->FindChildByName2<STreeFile>("addnew_tree_view");

		addnew_input_showmsg = _page->FindChildByName("addnew_input_showmsg");
		addnew_step_error = _page->FindChildByName("addnew_step_error");

		addnew_tree_view->BindStateText(addnew_filetree_state);
		_page->FindChildByName2<SButton>("addnew_btn_errretry")->GetEventSet()->subscribeEvent(EVT_LBUTTONUP, Subscriber(&UIAddNew::EvtBtnSubmiteClick, this));
		_page->FindChildByName2<SButton>("addnew_btn_errchance")->GetEventSet()->subscribeEvent(EVT_LBUTTONUP, Subscriber(&UIAddNew::EvtBtnChanceClick, this));


	}

	bool EvtInputLinkKeyDown(EventArgs* pEvt)
	{
		EventKeyDown* evt = (EventKeyDown*)pEvt;
		if (evt->nChar == 13) {//回车
			EvtBtnSubmiteClick(NULL);
		}
		return true;
	}

	void SwitchPageState(std::string state, wstring link = L"") {
		if (state == "show") {
			Link = link;
			addnew_div_linktip->SetVisible(true);
			addnew_div_step->SetVisible(false);
			addnew_div_tree->SetVisible(false);

			addnew_btn_submite->ModifyState(WndState_Normal, WndState_Disable, true);//启用 提交
			addnew_input_link->EnableWindow(true);//启用 输入链接

			addnew_input_link->SetWindowTextW(L"");//清空 输入链接

			addnew_input_showmsg->SetWindowTextW(L"");//清空 step提示
			addnew_step_error->SetVisible(false);
			addnew_tree_view->RemoveAllItems();
			addnew_tree_view->RefreshStateText();

			addnew_input_link->SetFocus();
			if (link != L"") {
				addnew_input_link->SetWindowTextW(Link.c_str());//自动填写 输入链接
				EvtBtnSubmiteClick(NULL);//自动点击提交链接
			}
			else {
				//尝试从剪切板自动读取
				wstring clipstr = OpenFileHelper::CopyFromClipboard();
				if (clipstr.length() >= 20 && clipstr.find(L"pan.baidu.com/s/") != wstring::npos) {
					addnew_input_link->SetWindowTextW(clipstr.c_str());//自动填写 输入链接

				}
			}

		}
		else if (state == "step") {//点击提交链接按钮后，显示step页面
			addnew_div_linktip->SetVisible(false);
			addnew_div_step->SetVisible(true);
			addnew_div_tree->SetVisible(false);

			addnew_btn_submite->ModifyState(WndState_Disable, WndState_Normal, true);//禁用 提交
			addnew_input_link->EnableWindow(false);//禁用 输入链接
			addnew_input_showmsg->SetWindowTextW(L"");//清空 step提示
			addnew_step_error->SetVisible(false);
			addnew_tree_view->RemoveAllItems();
			addnew_tree_view->RefreshStateText();

		}
		else if (state == "filetree") {//step 结束后 显示文件树
			addnew_div_linktip->SetVisible(false);
			addnew_div_step->SetVisible(false);
			addnew_div_tree->SetVisible(true);

			addnew_btn_submite->ModifyState(WndState_Disable, WndState_Normal, true);//禁用 提交
			addnew_input_link->EnableWindow(false);//禁用 输入链接
			addnew_input_showmsg->SetWindowTextW(L"");//清空 step提示
			addnew_step_error->SetVisible(false);

			//特殊
			std::wstring dir = UISetting::GetI()->GetDownDir();
			addnew_input_downdir->SetWindowTextW(dir.c_str());
		}
		_page->Invalidate();
	}
	void ShowFileTree(const vector<SerFileItem>& List) {
		//BDLink = bdlink;
		//FileList = List;
		SwitchPageState("filetree");
		//addnew_tree_view->GetClientRect();
		addnew_tree_view->RemoveAllItems();
		ShowFileTreeChild(STVI_ROOT, List);
		addnew_tree_view->RefreshStateText();
	}

	void ShowStepMsg(bool isEndError, std::wstring msg) {
		std::wstring s = addnew_input_showmsg->GetWindowTextW();
		if (s.length() == 0) s = L"进度：";
		s = s + L"\n" + msg;
		auto list = StringHelper::Split(s, L'\n');
		std::wstring n = L"进度：";
		int i = 0, min = list.size() > 6 ? list.size() - 6 : 0;
		for (auto iter = list.begin() + 1; iter != list.end(); iter++) {
			i++;
			if (i >= min) n = n + L"\n" + *iter;
		}
		list.clear();
		s = L"";
		addnew_input_showmsg->SetWindowTextW(n.c_str());
		n = L"";

		if (isEndError) addnew_step_error->SetVisible(true);
	}
private:

	void ShowFileTreeChild(HSTREEITEM parent, const vector<SerFileItem>& ChildList) {
		for (auto iter = ChildList.begin(); iter != ChildList.end(); iter++)
		{
			DowningItem Item = DowningItem();
			Item.share_fsid = iter->share_fsid;
			Item.local_mtime = iter->local_mtime;
			Item.path = iter->path;
			Item.server_filename = iter->server_filename;
			Item.size = iter->size;
			Item.sizestr = iter->sizestr;
			Item.down_timestr = iter->FileTime;//这里使用FileTime

			if (iter->isdir) {
				Item.sizestr = L"文件夹";
				Item.down_timestr = L"";
				HSTREEITEM pItem = addnew_tree_view->InsertItem(Item, true, parent, STVI_LAST, FALSE);
				if (iter->ChildList.size() > 0) ShowFileTreeChild(pItem, (*iter).ChildList);
			}
			else {
				addnew_tree_view->InsertItem(Item, false, parent, STVI_LAST, FALSE);
			}
		}
	}

	bool EvtBtnSubmiteClick(EventArgs* pEvt) {
		std::wstring linkstr = addnew_input_link->GetWindowTextW();
		if (linkstr == L"") return true;
		SwitchPageState("step");
		AppEvent::SendUI(UICmd::UIAddNew_SubmiteLink, 0, linkstr);
		return true;
	}
	bool EvtBtnRefreshClick(EventArgs* pEvt) {
		std::wstring linkstr = addnew_input_link->GetWindowTextW();
		if (linkstr == L"") return true;
		SwitchPageState("step");
		AppEvent::SendUI(UICmd::UIAddNew_SubmiteLink, 1, linkstr);
		return true;
	}
	bool EvtBtnChanceClick(EventArgs* pEvt) {
		//取消新建任务  隐藏 新增页面
		AppEvent::SendUI(UICmd::UIMain_ShowAddNew, 0, Link);
		return true;
	}

	bool EvtBtnSelectDirClick(EventArgs* pEvt) {
		wstring dir = UISetting::GetI()->SelectFolderW();
		if (dir == L"") return true;//没选择，直接返回
		addnew_input_downdir->SetWindowTextW(dir.c_str());
		UISetting::GetI()->SaveDownDir(dir);
		return true;
	}

	bool EvtBtnDownSelectClick(EventArgs* pEvt) {

		wstring DownDir = addnew_input_downdir->GetWindowTextW();
		if (DownDir == L"" || DownDir == L"未设置") {
			EvtBtnSelectDirClick(NULL);
			DownDir = addnew_input_downdir->GetWindowTextW();
			if (DownDir == L"" || DownDir == L"未设置") return true;
		}
		if (DownDir[DownDir.length() - 1] != L'\\') DownDir = DownDir + L"\\";
		//得到要下载的数据
		std::vector<DowningItem> List = std::vector<DowningItem>();
		List.reserve(1000);

		wstring down_timestr = S2W::StringtoWString(StringHelper::FormatJsTimeStr(StringHelper::GetTimeNow(), false), CP_UTF8);
		int64_t down_time = StringHelper::GetTimeNowHaoMiao() * 10000;//一次最多新增9999个文件
		size_t addcount = 0;
		HSTREEITEM hItem = addnew_tree_view->GetNextItem(STVI_ROOT);
		while (hItem)
		{
			LPTVITEMFILE pItem = CSTree<LPTVITEMFILE>::GetItem(hItem);
			if (pItem->bIsDir == false) {//文件图标
				if (pItem->nCheckBoxValue == STVICheckBox_Checked) {

					pItem->FileInfo.down_dir = DownDir;
					pItem->FileInfo.down_prog = 0;
					pItem->FileInfo.down_time = down_time++;
					pItem->FileInfo.down_state = L"waiting";
					pItem->FileInfo.down_timestr = down_timestr;
					List.emplace_back(pItem->FileInfo);
					addcount++;
					if (addcount > 9998) {
						SLOGFMTE(L"SelectTooManyFile share_fsid=%ws", pItem->FileInfo.share_fsid.c_str());
						break;
					}
				}
			}
			hItem = addnew_tree_view->GetNextItem(hItem);
		}
		List.shrink_to_fit();
		//保存到Downing
		UIDowning::GetI()->AddNew(List);

		//跳转回下载中
		AppEvent::SendUI(UICmd::UIMain_ShowAddNew, 0, Link);
		return true;
	}


};