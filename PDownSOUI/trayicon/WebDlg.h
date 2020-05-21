#pragma once
#include <souistd.h>
#include <string>
#include "SIECtrl.h"
namespace SOUI
{
    class WebDlg : public SHostDialog
    {
    public:
        WebDlg(void);
        ~WebDlg(void);
        BOOL OnInitDialog(HWND hWnd, LPARAM lParam);
        
        void SetTitle(std::wstring title);
        void SetTitle(std::wstring Title, std::wstring Url);
        void SetUrl(std::wstring Url);
        std::wstring title = L"";
        std::wstring url = L"";
        int Width = 0;
        int Height = 0;
        void SetSize(int width,int height);

        bool EvtBtnCloseClick(EventArgs* pEvt);

        SStatic* Title = nullptr;
        SIECtrl* IE = nullptr;

        BEGIN_MSG_MAP_EX(WebDlg)
            MSG_WM_INITDIALOG(OnInitDialog)
            CHAIN_MSG_MAP(SHostWnd)
        END_MSG_MAP()
    };

}