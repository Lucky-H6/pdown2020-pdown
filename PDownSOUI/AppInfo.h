#pragma once
#include "stdafx.h"
#include "Utils/singleton.h"
#include "vector"
#include "string"
#include "Utils/PathHelper.h"


class AppInfo
{
public:
	SINGLETON_DEFINE(AppInfo);
	AppInfo(void) {
		AppDir = PathHelper::GetLocalAppDataDir(APP_NAMEL);
	}
	~AppInfo(void) {

	}

public:
	std::wstring  AppDir = L"";
	int  ScreenWidth = 0;
	int  ScreenHeight = 0;
	bool IsDowning = false;
};