#include "stdafx.h"
#include "AppStart.h"
#include "Utils/CprHelper.h"
#include "Utils/PathHelper.h"
#include "Utils/AppThread.h"
#include "AppEvent.h"
#include "Utils/ZlibFileHelper.h"
#include "DAL/DBHelper.h"
#include "Utils/unzip.h"
#include "AppInfo.h"
#include "Utils/Blink.h"


//自定义的初始化
CURLSH* CprHelper::DNSShared;
ThreadPool AppThread::HttpPool(5);
ThreadPool AppThread::DBPool(1);
ThreadPool AppThread::BlinkPool(1);
SNotifyCenter* AppEvent::pNotifyCenter;

//自定义的初始化结束


HANDLE AppStart::hMutex = NULL;
bool AppStart::checkMyselfExist()
{
	hMutex = CreateMutex(NULL, TRUE, APP_NAMEL);
	DWORD dwRet = GetLastError();
	if (dwRet == ERROR_ALREADY_EXISTS)
	{
		if (hMutex) CloseHandle(hMutex);
		hMutex = nullptr;


		HWND  win = ::FindWindow(APP_NAMEL, APP_NAMEL);
		if (win) {
			while (true) {//循环查找，找到最顶级窗口
				long hwl2p = ::GetWindowLong(win, GWL_HWNDPARENT);
				if (hwl2p != 0) {//返回0 说明失败，没有父窗口  非0则是父窗口句柄
					win = (HWND)hwl2p;
				}
				else {
					break;
				}
			}
			::SendMessage(win, WM_COMMAND, 1, NULL);//这里1 是对应OnCommand(0,1,null)
			//ShowWindow(win, SW_SHOW);
		}
		return true;
	}
	else {
		return false;
	}
}


/*检测是否是从local目录启动*/
bool AppStart::checkRunFromLocal() {

#ifndef _DEBUG	
	std::wstring AppDir = AppInfo::GetI()->AppDir;
	std::wstring exe_local = AppDir + APP_NAMEL + L".exe";
	std::wstring exe_localnew = AppDir + APP_NAMEL + L"_new.exe";
	std::wstring exe_start = PathHelper::GetAppStartDir();
	if (_waccess(exe_localnew.c_str(), 0) == 0) {//如果存在新版本
		_wremove(exe_local.c_str());//删除旧的
		MoveFile(exe_localnew.c_str(), exe_local.c_str());//复制新的
	}
	if (exe_local != exe_start) {
		if (_waccess(exe_local.c_str(), 0) != 0) {//如果不存在local
			CopyFileW(exe_start.c_str(), exe_local.c_str(), true);//复制start到local
		}

		if (_waccess(exe_local.c_str(), 0) == 0) {//最后判断，如果存在local就运行local
			exe_local = L"\"" + exe_local + L"\"";
			ShellExecuteW(NULL, L"open", exe_local.c_str(), NULL, NULL, SW_SHOWDEFAULT);
			return true;//只有最终需要从local重启，才返回true，导致程序直接退出
		}
	}
#endif
	return false;
}

/*备份日志文件*/
bool AppStart::checkBackUpLog() {

	std::wstring AppDir = AppInfo::GetI()->AppDir;
	std::wstring log7z = AppDir + L"log.txt";
	ZlibFileHelper ZLib = ZlibFileHelper();
	if (!ZLib.CreatZlibFile(log7z)) return false;

	bool IsAddFile = false;
	for (int i = 0; i < 10; i++) {
		std::wstring logfile = AppDir + APP_NAMEL + L"-" + to_wstring(i) + L".log";
		if (_waccess(logfile.c_str(), 0) == 0) {//如果存在日志文件
			if (ZLib.PackOneFile(logfile)) {
				IsAddFile = true;
				_wremove(logfile.c_str());
			}
		}
	}
	ZLib.CloseZlibFile();

	return true;

}




void AppStart::AppStartInit(SApplication* m_theApp)
{
	DBHelper::GetI()->OpenDBInit(AppInfo::GetI()->AppDir + L"data.db");
	CprHelper::GobalInit();
	AppEvent::InitAppEvent();


	//修正右键菜单
	wstring DPI = DBHelper::GetI()->GetConfigData("DPI");
	SStringT editmenuxml = _T("LAYOUT:XML_EDITMENU");
	if (DPI == L"150") {
		editmenuxml = _T("LAYOUT:XML_EDITMENU_150");
	}
	pugi::xml_document xmlDoc;
	if (m_theApp->LoadXmlDocment(xmlDoc, editmenuxml))
	{
		SRicheditMenuDef::getSingleton().SetMenuXml(xmlDoc.child(L"editmenu"));
	}
	Blink::GetI()->Run_OpenNode();
}

void AppStart::AppCloseClean()
{
	Blink::GetI()->Run_CloseNode();
	AppThread::WaitUntilStop();
	CprHelper::GobalClean();
	AppEvent::DestroyAppEvent();
	DBHelper::GetI()->CloseDBFile();

}
