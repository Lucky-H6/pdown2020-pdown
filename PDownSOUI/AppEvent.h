#pragma once
#include "stdafx.h"
#include <string>
#include "event/SNotifyCenter.h"

enum UICmd {
	null,
	UIMain_ShowAddNew,
	UIHome_SearchByUser,
	UIHome_SubmiteSearch,
	UIHome_CheckUP,
	UIAddNew_SubmiteLink,
	UIDowning_UpdateDownItemState,
	UIDowning_DeleteDownItem,
	UIDowning_FireDownloader,
	UIDowning_RunOneDownload,
	UIDowned_DeleteDownItem,
	UIDowned_AddDownItem,
	UIUser_SendCode,
	UIUser_Login
};
//演示使用SNotifyCenter的同步事件
class EventUI : public TplEventArgs<EventUI>
{
	SOUI_CLASS_NAME(EventUI, L"on_event_ui")
public:
	EventUI(SObject* pSender) :TplEventArgs<EventUI>(pSender) {}
	enum { EventID = EVT_EXTERNAL_BEGIN + 300001 };
	UICmd cmd = UICmd::null;
	int dataint = 0;
	std::wstring datastr = L"";
};


class AppEvent {
public:
	static SNotifyCenter* pNotifyCenter;
public:
	static void InitAppEvent() {
		pNotifyCenter = new SNotifyCenter;
		pNotifyCenter->addEvent(EVENTID(EventUI));
	}
	static void DestroyAppEvent() {
		if (pNotifyCenter) {
			delete pNotifyCenter;
			pNotifyCenter = nullptr;
		}
	}

	static void SendUI(UICmd cmd, int dataint, std::wstring datastr) {
		EventUI* m = new EventUI(nullptr);
		m->cmd = cmd;
		m->dataint = dataint;
		m->datastr = datastr;
		if (pNotifyCenter)pNotifyCenter->FireEventAsync(m);
		m->Release();
	}
};