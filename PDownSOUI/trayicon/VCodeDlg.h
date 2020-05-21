#pragma once
#include <souistd.h>
#include <string>
namespace SOUI
{
    class VCodeDlg : public SHostDialog
    {
    public:
        VCodeDlg(void);
        ~VCodeDlg(void);
        BOOL OnInitDialog(HWND hWnd, LPARAM lParam);
        
        void SetImageUrl(std::wstring url);
        void ShowImage();
        std::wstring img_url = L"";
        std::wstring txt_code = L"";

        bool EvtBtnCloseClick(EventArgs* pEvt);
        bool EvtLinkChangeClick(EventArgs* pEvt);
        bool EvtBtnCodeClick(EventArgs* pEvt);

        SButton* btn_code = nullptr;
        SEdit* input_code = nullptr;
        SWindow* link_change = nullptr;
        SImageWnd* img_code = nullptr;

        BEGIN_MSG_MAP_EX(VCodeDlg)
            MSG_WM_INITDIALOG(OnInitDialog)
            CHAIN_MSG_MAP(SHostWnd)
        END_MSG_MAP()
    };

}