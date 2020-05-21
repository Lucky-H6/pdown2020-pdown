#pragma once
#include <string>
class AppStart
{
	
public:
	static int ScreenWidth;
	static int ScreenHeight;

	static HANDLE  hMutex;
	/*如果程序已经有一个在运行，则返回true*/
	static bool  checkMyselfExist();

	/*检测是否是从local目录启动*/
	static bool checkRunFromLocal();

	/*备份日志文件*/
	static bool checkBackUpLog();

	static void AppStartInit(SApplication* m_theApp);
	static void AppCloseClean();
};

