#pragma once
#include "../stdafx.h"
#include "singleton.h"
#include "vector"
#include "string"
#include "wke.h"
#include "../AppInfo.h"
#include "unzip.h"
#include "AppThread.h"

class Blink
{
public:
	SINGLETON_DEFINE(Blink);
	Blink(void) {

	}
	~Blink(void) {

	}
private:
	bool isNodeDll = false;
	wkeWebView blink = nullptr;
	wstring UserAgent = L"";

public:
	/*导出node.dll*/
	bool checkNodeDll() {
		if (isNodeDll) return true;

		std::wstring appdir = AppInfo::GetI()->AppDir;
		std::wstring nodedllfile = appdir + L"node.dat";
		if (_waccess(nodedllfile.c_str(), 0) != 0)//文件不存在
		{
			SApplication* m_theApp = SApplication::getSingletonPtr();
			if (!m_theApp) return false;
			size_t nodelen = m_theApp->GetRawBufferSize(L"raw", L"node");
			if (nodelen > 0) {
				unsigned char* buff = (unsigned char*)malloc(nodelen + 1);
				try {
					if (m_theApp->GetRawBuffer(L"raw", L"node", buff, nodelen)) {

						HZIP hz = OpenZip(buff, nodelen, NULL);
						ZIPENTRY ze;
						GetZipItem(hz, -1, &ze);
						int numitems = ze.index;
						for (int i = 0; i < numitems; i++)
						{
							GetZipItem(hz, i, &ze);
							if (std::wstring(ze.name) == L"node.dll")
							{
								UnzipItem(hz, i, nodedllfile.c_str());
							}
						}
						CloseZip(hz);
					}
				}
				catch (...) {}
				free(buff);
				buff = nullptr;
			}
		}

		if (_waccess(nodedllfile.c_str(), 0) == 0) {
			isNodeDll = true;
			wkeSetWkeDllPath(nodedllfile.c_str());
			BingSetting();
			BingCallback();
			return true;
		}

		return false;
	}

	void BingSetting();
	void BingCallback();

public:
	void Run_OpenNode() {
		//AppThread::BlinkPool.enqueue([]() {	Blink::GetI()->checkNodeDll(); });
		bool isok = false;
		try {
			isok = checkNodeDll();
		}
		catch (...) {}
		if (isok) {
			wkeProxy proxy;
			strcpy(proxy.hostname, "127.0.0.1");
			proxy.port = 8888;
			strcpy(proxy.username, "");
			strcpy(proxy.password, "");
			proxy.type = wkeProxyType::WKE_PROXY_HTTP;
			wkeSetViewProxy(blink, &proxy);
		}
	}
	void Run_CloseNode() {
		try {
			if (blink != nullptr) wkeDestroyWebWindow(blink);
		}
		catch (...) {}

		try
		{
			wkeShutdown();
			wkeFinalize();
		}
		catch (...)
		{
			
		}
		
	}
	void Run_GoUrl(std::wstring url) {
		if (!blink) return;
		wkeLoadURLW(blink, url.c_str());
	}

	void Run_UserAgent(std::wstring useragent) {
		if (!blink) return;
		if (useragent == L"") return;
		if (UserAgent == L"") {
			UserAgent = useragent;
			wkeSetUserAgentW(blink, useragent.c_str());
		}
	}

	void Run_JS(std::wstring js) {
		if (!blink) return;
		wkeRunJSW(blink, js.c_str());
	}

	void Run_Mouse(int x, int y) {
		if (!blink) return;
		wkeFireMouseEvent(blink, WKE_MSG_LBUTTONDOWN, x, y, 0);
		this_thread::sleep_for(std::chrono::milliseconds(100));
		wkeFireMouseEvent(blink, WKE_MSG_LBUTTONUP, x, y, 0);
	}
	void Run_Mouse(wkeMouseMsg msg, int x, int y) {
		if (!blink) return;
		wkeFireMouseEvent(blink, msg, x, y, 0);
	}


};