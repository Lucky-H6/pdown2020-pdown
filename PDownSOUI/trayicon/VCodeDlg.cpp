#include "StdAfx.h"
#include "VCodeDlg.h"
#include "../Utils/CprHelper.h"

namespace SOUI
{
	VCodeDlg::VCodeDlg(void) :SHostDialog(_T("layout:dlg_vcode"))
	{

	}

	VCodeDlg::~VCodeDlg(void)
	{
	}
	BOOL VCodeDlg::OnInitDialog(HWND hWnd, LPARAM lParam)
	{

		FindChildByName(L"btn_close")->GetEventSet()->subscribeEvent(EVT_LBUTTONUP, Subscriber(&VCodeDlg::EvtBtnCloseClick, this));

		img_code = FindChildByName2<SImageWnd>(L"img_code");

		link_change = FindChildByName(L"link_change");
		link_change->GetEventSet()->subscribeEvent(EVT_LBUTTONUP, Subscriber(&VCodeDlg::EvtLinkChangeClick, this));
		btn_code = FindChildByName2<SButton>(L"btn_code");
		btn_code->GetEventSet()->subscribeEvent(EVT_LBUTTONUP, Subscriber(&VCodeDlg::EvtBtnCodeClick, this));
		input_code = FindChildByName2<SEdit>(L"input_code");
		input_code->SetFocus();
		txt_code = L"";
		BringWindowToTop();
		ShowImage();
		return 0;
	}
	void VCodeDlg::SetImageUrl(std::wstring url) {
		img_url = url;
		
	}

	bool VCodeDlg::EvtBtnCloseClick(EventArgs* pEvt) {
		txt_code = L"";
		EndDialog(IDCANCEL);
		return true;
	}
	bool VCodeDlg::EvtLinkChangeClick(EventArgs* pEvt) {
		ShowImage();
		return true;
	}

	bool VCodeDlg::EvtBtnCodeClick(EventArgs* pEvt) {
		txt_code = input_code->GetWindowTextW();
		while (txt_code.length() > 0) {
			if (txt_code[0] == L' ') txt_code = txt_code.substr(1);
			else break;
		}
		while (txt_code.length() > 0) {
			int len = txt_code.length() - 1;
			if (txt_code[len] == L' ') txt_code = txt_code.substr(0, len - 1);
			else break;
		}
		if (txt_code.length() != 4) txt_code = L"";
		EndDialog(IDOK);
		return true;
	}

	/*网络拉取验证码图片并显示*/
	void VCodeDlg::ShowImage() {

		if (img_url != L"" && img_url.find(L"http") != wstring::npos) {
			SLOGFMTE(L"ShowImage img_url = %ws", img_url.c_str());
			auto resp = CprHelper::Get3(img_url, L"X-Requested-With: XMLHttpRequest\n", true);
			if (resp.IsStatus200()) {
				unsigned char* buff = (unsigned char*)resp.backdata.data();
				size_t len = resp.backdata.length();
				SAutoRefPtr<IBitmap> bmp;
				GETRENDERFACTORY->CreateBitmap(&bmp);
				HRESULT r = bmp->LoadFromMemory(buff, len);
				if (r == S_OK) {
					img_code->SetImage(bmp, FilterLevel::kHigh_FilterLevel);
					return;
				}
				else {
					SLOGFMTE(L"ShowImage loaderror =%hs", resp.backheader.c_str());
				}
			}
			else {
				SLOGFMTE(L"ShowImage DownError =%hs", resp.backheader.c_str());
			}
			img_code->SetImage(nullptr);
		}
	}
}