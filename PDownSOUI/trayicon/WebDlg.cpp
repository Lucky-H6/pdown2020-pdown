#include "StdAfx.h"
#include "WebDlg.h"

namespace SOUI
{
	WebDlg::WebDlg(void) :SHostDialog(_T("layout:dlg_webdlg"))
	{

	}

	WebDlg::~WebDlg(void)
	{
	}
	BOOL WebDlg::OnInitDialog(HWND hWnd, LPARAM lParam)
	{
		if (this->Width > 0) {
			EventSwndSize ev = EventSwndSize(this);
			ev.szWnd.SetSize(this->Width, this->Height);
			this->FireEvent(ev);
		}
		FindChildByName(L"btn_close")->GetEventSet()->subscribeEvent(EVT_LBUTTONUP, Subscriber(&WebDlg::EvtBtnCloseClick, this));

		Title = FindChildByName2<SStatic>(L"title");
		IE = FindChildByName2<SIECtrl>(L"ie");

		Title->SetWindowTextW(title.c_str());
		IE->Navigate(url.c_str());
		//this->CenterWindow();
		return 0;
	}
	void WebDlg::SetTitle(std::wstring Title) {
		this->title = Title;
		if(this->Title!=nullptr) this->Title->SetWindowTextW(title.c_str());
		//
	}
	void WebDlg::SetTitle(std::wstring Title, std::wstring Url) {
		this->title = Title;
		if (this->Title != nullptr) this->Title->SetWindowTextW(title.c_str());
		this->url = Url;
		if (this->IE != nullptr) {
			this->IE->Navigate(url.c_str());
		}
	}
	void WebDlg::SetUrl(std::wstring Url) {
		this->url = Url;
		if (this->IE != nullptr) this->IE->Navigate(url.c_str());
	}
	void WebDlg::SetSize(int width, int height)
	{
		this->Width = width;
		this->Height = height;
	}
	bool WebDlg::EvtBtnCloseClick(EventArgs* pEvt) {
		EndDialog(IDOK);
		return true;
	}

}