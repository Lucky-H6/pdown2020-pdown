#pragma once
#include "../AppEvent.h"
#include "../DAL/DALAddNew.h"	
#include "../DAL/DALHome.h"	
#include "../DAL/DALUser.h"	
#include "../UIPage/UIDowning.h"	
#include "../UIPage/UIDowned.h"	
#include "../UIPage/UIHome.h"	

#include <thread>
class UIAppEvent {
public:
	static void OnUIEvent(EventUI* e) {
		switch (e->cmd)
		{
		case UICmd::UIMain_ShowAddNew: {
			bool IsShow = (e->dataint == 1);
			UIAddNew::GetI()->SwitchPageState("show", (IsShow ? e->datastr : L""));//不管是显示还是隐藏，都清理一下数据
			UIMain::GetI()->ShowAddNew(IsShow, e->datastr);
			break;
		}
		case UICmd::UIHome_SearchByUser: {
			UIHome::GetI()->SearchByUser(e->datastr);
			break;
		}
		case UICmd::UIHome_SubmiteSearch: {
			AppThread::HttpPool.enqueue([](std::wstring key) {DALHome::SubmiteSearch(key);	}, e->datastr);
			break;
		}
		case UICmd::UIHome_CheckUP: {
			AppThread::HttpPool.enqueue([]() {DALHome::CheckUP();	});
			AppThread::HttpPool.enqueue([]() {DALHome::PostLog();	});
			break;
		}
		case UICmd::UIAddNew_SubmiteLink: {
			AppThread::HttpPool.enqueue([](std::wstring linkstr, bool isrefresh) {DALAddNew::SubmiteLink(linkstr, isrefresh);	}, e->datastr, (e->dataint == 1));
			break;
		}
		case UICmd::UIDowning_UpdateDownItemState: {
			UIDowning::GetI()->UpdateDownItemState(e->datastr);
			break;
		}
		case UICmd::UIDowning_DeleteDownItem: {
			UIDowning::GetI()->DeleteDownItem(e->datastr);
			break;
		}
		case UICmd::UIDowning_FireDownloader: {
			UIDowning::GetI()->FireDownloader();
			break;
		}
		case UICmd::UIDowned_AddDownItem: {
			UIDowned::GetI()->AddDownItem(e->datastr);
			break;
		}
		case UICmd::UIUser_SendCode: {
			AppThread::HttpPool.enqueue([](std::wstring phone) {DALUser::SendCode(phone);	}, e->datastr);
			break;
		}
		case UICmd::UIUser_Login: {
			AppThread::HttpPool.enqueue([](std::wstring phone, std::wstring code) {DALUser::Login(phone, code);	}, e->datastr, to_wstring(e->dataint));
			break;
		}
		default:break;
		}
		//e->Release();
	}
};