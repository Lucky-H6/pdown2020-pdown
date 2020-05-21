#pragma once
#include <string>
#include "../Utils/singleton.h"
#include <souistd.h>
#include "UIMain.h"	
#include "../trayicon/WebDlg.h"
#include "../trayicon/VCodeDlg.h"
using namespace SOUI;


#include <windows.h>
#include <shobjidl.h> 

class UISetting
{
public:
	SINGLETON_DEFINE(UISetting);
	UISetting(void) {}
	~UISetting(void) {}
private:
	SWindow* _page = nullptr;
	SWindow* setting_input_downdir = nullptr;
	SComboBox* setting_cmb_dpi = nullptr;
	std::wstring DownDir = L"";
public:
	void InitPage(SOUI::SWindow* main) {
		_page = main;

		_page->FindChildByName("setting_btn_selectdir")->GetEventSet()->subscribeEvent(EVT_LBUTTONUP, Subscriber(&UISetting::EvtBtnSelectDirClick, this));
		_page->FindChildByName("setting_about")->GetEventSet()->subscribeEvent(EVT_LBUTTONUP, Subscriber(&UISetting::EvtLinkAboutClick, this));
		_page->FindChildByName("setting_help")->GetEventSet()->subscribeEvent(EVT_LBUTTONUP, Subscriber(&UISetting::EvtLinkHelpClick, this));
		_page->FindChildByName("setting_report")->GetEventSet()->subscribeEvent(EVT_LBUTTONUP, Subscriber(&UISetting::EvtLinkReportClick, this));
		_page->FindChildByName("setting_bug")->GetEventSet()->subscribeEvent(EVT_LBUTTONUP, Subscriber(&UISetting::EvtLinkBugClick, this));
		_page->FindChildByName("setting_dpi")->GetEventSet()->subscribeEvent(EVT_LBUTTONUP, Subscriber(&UISetting::EvtLinkDPIClick, this));

		setting_cmb_dpi = _page->FindChildByName2<SComboBox>("setting_cmb_dpi");
		auto DPI = DBHelper::GetI()->GetConfigData("DPI");
		if (DPI == L"150") setting_cmb_dpi->SetCurSel(1);
		else setting_cmb_dpi->SetCurSel(0);
		setting_cmb_dpi->GetEventSet()->subscribeEvent(EVT_CB_SELCHANGE, Subscriber(&UISetting::EvtCBDPIChanged, this));

		DownDir = DBHelper::GetI()->GetConfigData("DownPath");
		if (DownDir == L"") DownDir = L"未设置";
		setting_input_downdir = _page->FindChildByName(L"setting_input_downdir");
		setting_input_downdir->SetWindowTextW(DownDir.c_str());

	}
	/*显示选择文件夹窗口*/
	std::wstring SelectFolderW()
	{
		wchar_t dirpath[_MAX_PATH];

		PWSTR pszPath;
		DWORD dwOptions;
		IShellItem* pItem;
		IFileDialog* pFile;
		HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED |
			COINIT_DISABLE_OLE1DDE);
		if (FAILED(hr)) {
			return L"";
		}
		hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
			IID_PPV_ARGS(&pFile));
		if (FAILED(hr)) {
			CoUninitialize();
			return L"";
		}
		bool isok = false;
		do {
			hr = pFile->GetOptions(&dwOptions);
			if (FAILED(hr)) {
				break;
			}
			pFile->SetOptions(dwOptions | FOS_PICKFOLDERS);
			hr = pFile->Show(NULL);
			if (FAILED(hr)) {
				break;
			}
			hr = pFile->GetResult(&pItem);
			if (FAILED(hr)) {
				break;
			}
			hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszPath);
			if (FAILED(hr)) {
				break;
			}
			wcscpy(dirpath, pszPath);
			CoTaskMemFree(pszPath);
			isok = true;
		} while (0);
		pFile->Release();
		CoUninitialize();
		std::wstring dir = std::wstring(dirpath);
		if (dir[dir.length() - 1] != L'\\') dir = dir + L"\\";
		if (isok) return dir;
		else return L"";
	}
	/*保存下载目录*/
	void SaveDownDir(std::wstring dir) {
		AppThread::DBPool.enqueue([dir]() {
			DBHelper::GetI()->SaveConfigData("DownPath", dir);
			});
		DownDir = dir;
		setting_input_downdir->SetWindowTextW(DownDir.c_str());
	}
	std::wstring GetDownDir() {
		return DownDir;
	}

private:

	void ShowDlg(wstring title, wstring url) {
		try {
			WebDlg web;
			auto rec = _page->GetRoot()->GetWindowRect();
			int w = rec.Width();//1080  780
			int h = rec.Height();//720
			if (w > 1080) w = 1080;
			if (h > 720) h = 720;
			web.SetSize(w, h);
			web.SetTitle(title, ServerIP + url);
			web.DoModal();
		}
		catch (...) {}
	}


	bool EvtBtnSelectDirClick(EventArgs* pEvt) {
		std::wstring dir = SelectFolderW();
		if (dir == L"") return true;//没选择，直接返回
		SaveDownDir(dir);
		return true;
	}
	bool EvtLinkAboutClick(EventArgs* pEvt) {
		ShowDlg(L"关于我们", L"about.html");
		return true;
	}
	bool EvtLinkYuanLiClick(EventArgs* pEvt) {

		return true;
	}
	bool EvtLinkHelpClick(EventArgs* pEvt) {
		//https://github.com/pdown2020/pdown
		return true;
	}

	bool EvtCBDPIChanged(EventArgs* pEvt) {
		EventCBSelChange* e2 = sobj_cast<EventCBSelChange>(pEvt);
		SComboBox* pCbx = sobj_cast<SComboBox>(e2->sender);
		if (e2->nCurSel != -1)
		{
			auto data = pCbx->GetItemData(e2->nCurSel);
			if (data == 150) {//切换到150%
				DBHelper::GetI()->SaveConfigData("DPI", L"150");
			}
			else {//切换到100%
				DBHelper::GetI()->SaveConfigData("DPI", L"100");
			}
			_page->FindChildByName("setting_txt_dpi")->SetVisible(true, true);
		}
		return true;
	}

public:
	bool EvtLinkReportClick(EventArgs* pEvt) {
		ShowDlg(L"侵权违规文件举报", L"report.html");
		return true;
	}
	bool EvtLinkVerClick(std::wstring ver) {
		ShowDlg(L"版本更新", L"ver.html?t=" + to_wstring(StringHelper::GetTimeNow()));
		return true;
	}
	bool EvtLinkDPIClick(EventArgs* pEvt) {
		EvtLinkVerClick(APP_VER);
		return true;
	}
private:
	bool EvtLinkBugClick(EventArgs* pEvt) {
		//https://github.com/pdown2020/pdown/issues
		return true;
	}

};