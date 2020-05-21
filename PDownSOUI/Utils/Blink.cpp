#include "stdafx.h"
#include "Blink.h"

wkeWebView wkeCreateViewCallback2(wkeWebView webView, void* param, wkeNavigationType navigationType, const wkeString url, const wkeWindowFeatures* windowFeatures)
{
	return webView;
}

void wkeAlertBoxCallback2(wkeWebView webView, void* param, const wkeString msg) {

}

bool  wkeConfirmBoxCallback2(wkeWebView webView, void* param, const wkeString msg) {
	return false;
}
bool wkePromptBoxCallback2(wkeWebView webView, void* param, const wkeString msg, const wkeString defaultResult, wkeString result) {
	return false;
}

bool wkeDownloadCallback2(wkeWebView webView, void* param, const char* url) {
	return false;
}
void Blink::BingSetting() {

	std::wstring appdir = AppInfo::GetI()->AppDir;
	std::wstring ls = appdir + L"LocalStorage\\";
	if (_waccess(ls.c_str(), 0) != 0) PathHelper::CreatDir(ls);

	wkeInitialize();
	int w = AppInfo::GetI()->ScreenWidth;
	int h = AppInfo::GetI()->ScreenHeight;
	if (w > 1980) w = 1980;
	if (w < 1024) w = 1024;
	if (h > 1080) h = 1080;
	if (h < 700) h = 700;

	//blink = wkeCreateWebView();// wkeCreateWebWindow(WKE_WINDOW_TYPE_TRANSPARENT, NULL, 0, 0, w, h);
	//wkeResizeWindow(blink, w, h);
	blink = wkeCreateWebWindow(WKE_WINDOW_TYPE_TRANSPARENT, NULL, 0, 0, w, h);
	try
	{
		wkeSetHeadlessEnabled(blink, true);
		wkeSetNavigationToNewWindowEnable(blink, false);
		wkeSetLocalStorageFullPath(blink, ls.c_str());
		wkeSetCookieJarPath(blink, ls.c_str());
		wkeSetMediaVolume(blink, 0);
	}
	catch (...) {}

}
void Blink::BingCallback() {
	wkeOnAlertBox(blink, wkeAlertBoxCallback2, this);
	wkeOnConfirmBox(blink, wkeConfirmBoxCallback2, this);
	wkeOnPromptBox(blink, wkePromptBoxCallback2, false);
	wkeOnCreateView(blink, wkeCreateViewCallback2, this);
	wkeOnDownload(blink, wkeDownloadCallback2, this);
}