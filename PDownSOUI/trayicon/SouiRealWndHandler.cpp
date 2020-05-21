#include "StdAfx.h"
#include "SouiRealWndHandler.h"
#include "WebDlg.h"
#include <souistd.h>

namespace SOUI
{
    CSouiRealWndHandler::CSouiRealWndHandler(void)
    {
    }

    CSouiRealWndHandler::~CSouiRealWndHandler(void)
    {
    }

    HWND CSouiRealWndHandler::OnRealWndCreate(SRealWnd* pRealWnd)
    {
        const SRealWndParam& param = pRealWnd->GetRealWndParam();
        if (param.m_strClassName == _T("WebDlg"))
        {
            WebDlg* pWnd = new WebDlg;
            //mfc.demo中的例子 pbtn->Create(param.m_strWindowName,WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,::CRect(0,0,0,0),CWnd::FromHandle(pRealWnd->GetContainer()->GetHostHwnd()),pRealWnd->GetID());
            pWnd->Create(pRealWnd->GetContainer()->GetHostHwnd(), WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, 0);
            pRealWnd->SetData(pWnd);
            //返回成功创建后的窗口句柄
            return pWnd->m_hWnd;
        }
        else
        {
            return 0;
        }
    }

    void CSouiRealWndHandler::OnRealWndDestroy(SRealWnd* pRealWnd)
    {
        const SRealWndParam& param = pRealWnd->GetRealWndParam();
        if (param.m_strClassName == _T("WebDlg"))
        {//销毁真窗口，释放窗口占用的内存
            WebDlg* pWnd = (WebDlg*)pRealWnd->GetData();
            if (pWnd)
            {
                pWnd->DestroyWindow();
                delete pWnd;
            }
        }
    }

    //不处理，返回FALSE
    BOOL CSouiRealWndHandler::OnRealWndSize(SRealWnd* pRealWnd)
    {
        return FALSE;
    }

    //不处理，返回FALSE
    BOOL CSouiRealWndHandler::OnRealWndInit(SRealWnd* pRealWnd)
    {
        return FALSE;
    }
}